//! Compute shader for the CD-I.
//! The caller is responsible for calling the appropriate shader entry point.
//!
//! There is no bounds checking in the kernels because the workgroupe size is 64,
//! which is a divisor of all the possible screen resolutions.
//!
//! Buffer commentated as `Height` means they contain one value per line, all other buffers contain one value per pixel.

@group(0) @binding(0) var<storage, read_write> screen: array<u32>;
@group(0) @binding(1) var<storage, read_write> plane_a: array<u32>;
@group(0) @binding(2) var<storage, read_write> plane_b: array<u32>;
@group(0) @binding(3) var<storage, read> background: array<u32>; // Height.

// All the buffers below are actually buffers of u8, in little endian.
@group(0) @binding(4) var<storage, read> transparency_a: array<u32>; // Height.
@group(0) @binding(5) var<storage, read> transparency_b: array<u32>; // Height.
@group(0) @binding(6) var<storage, read> mask_color_a: array<u32>; // Height, u32.
@group(0) @binding(7) var<storage, read> mask_color_b: array<u32>; // Height, u32.
@group(0) @binding(8) var<storage, read> transparent_color_a: array<u32>; // Height, u32.
@group(0) @binding(9) var<storage, read> transparent_color_b: array<u32>; // Height, u32.
@group(0) @binding(10) var<storage, read> matte_flag_a: array<u32>;
@group(0) @binding(11) var<storage, read> matte_flag_b: array<u32>;
@group(0) @binding(12) var<storage, read> icf_a: array<u32>;
@group(0) @binding(13) var<storage, read> icf_b: array<u32>;

@group(0) @binding(14) var<uniform> width: u32; // The width of the image in pixels.

///////////////////////////// Transparency /////////////////////////////////////

/// Returns the transparent control A at the given index.
fn get_transparent_a(index: u32) -> u32 {
    let i = index / width;
    let idx = i / 4; // sizeof(u32)
    let shift = (i % 4) * 8;
    return transparency_a[idx] >> shift & 0xFF;
}

/// Returns the transparent control B at the given index.
fn get_transparent_b(index: u32) -> u32 {
    let i = index / width;
    let idx = i / 4; // sizeof(u32)
    let shift = (i % 4) * 8;
    return transparency_b[idx] >> shift & 0xFF;
}

/// Returns the mask color A at the given index.
fn get_mask_color_a(index: u32) -> u32 {
    let idx = index / width;
    return mask_color_a[idx];
}

/// Returns the mask color B at the given index.
fn get_mask_color_b(index: u32) -> u32 {
    let idx = index / width;
    return mask_color_b[idx];
}

/// Returns the transparent color A at the given index.
fn get_transparent_color_a(index: u32) -> u32 {
    let idx = index / width;
    return transparent_color_a[idx];
}

/// Returns the transparent color B at the given index.
fn get_transparent_color_b(index: u32) -> u32 {
    let idx = index / width;
    return transparent_color_b[idx];
}

/// Returns the matte flag A at the given index.
fn get_matte_a(index: u32) -> u32 {
    let idx = index / 4; // sizeof(u32)
    let shift = (index % 4) * 8;
    return matte_flag_a[idx] >> shift & 0xFFu;
}

/// Returns the matte flag B at the given index.
fn get_matte_b(index: u32) -> u32 {
    let idx = index / 4; // sizeof(u32)
    let shift = (index % 4) * 8;
    return matte_flag_b[idx] >> shift & 0xFFu;
}

/// Masks the given color to the actually used bytes (V.5.7.2.2).
fn clut_color_key(color: u32) -> u32 {
    return color & 0x00FCFCFC;
}

/// Computes the transparency for both planes.
fn handle_transparency(pixel: u32, transparent: u32, mask_color: u32, transparent_color: u32, matte_a: u32, matte_b: u32) -> u32 {
    let color = clut_color_key(pixel | mask_color);
    let color_key = color == clut_color_key(transparent_color | mask_color);

    let boolean = (transparent & 8) == 0;
    var hidden = false;

    switch transparent & 7 {
        case 0: { // Always/Never.
            hidden = boolean;
        }
        case 1: { // Color Key.
            hidden = color_key == boolean;
        }
        case 2: { // Transparent Bit.
            hidden = ((pixel & 0xFF000000) == 0xFF000000) != boolean;
        }
        case 3: { // Matte Flag 0.
            hidden = matte_a == u32(boolean);
        }
        case 4: { // Matte Flag 1.
            hidden = matte_b == u32(boolean);
        }
        case 5: { // Matte Flag 0 or Color Key.
            hidden = (matte_a == u32(boolean)) || (color_key == boolean);
        }
        case 6: { // Matte Flag 1 or Color Key.
            hidden = (matte_b == u32(boolean)) || (color_key == boolean);
        }
        default: {
            hidden = true;
        }
    }

    if hidden {
        return pixel & 0x00FFFFFF;
    } else {
        return pixel | 0xFF000000;
    }
}

/// Computes transparency for both planes.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn transparency(@builtin(global_invocation_id) id: vec3u) {
    plane_a[id.x] = handle_transparency(plane_a[id.x], get_transparent_a(id.x), get_mask_color_a(id.x),
                                        get_transparent_color_a(id.x), get_matte_a(id.x), get_matte_b(id.x));
    plane_b[id.x] = handle_transparency(plane_b[id.x], get_transparent_b(id.x), get_mask_color_b(id.x),
                                        get_transparent_color_b(id.x), get_matte_a(id.x), get_matte_b(id.x));
}

///////////////////////////// Overlay/mixing ///////////////////////////////////

/// Returns the value of IFC front for the given pixel index.
///
/// This is required because WGSL doesn't have a u8 type.
fn get_icf_a(index: u32) -> u32 {
    let idx = index / 4; // sizeof(u32)
    let shift = (index % 4) * 8;
    return icf_a[idx] >> shift & 0xFFu;
}

/// Returns the value of IFC back for the given pixel index.
///
/// This is required because WGSL doesn't have a u8 type.
fn get_icf_b(index: u32) -> u32 {
    let idx = index / 4; // sizeof(u32)
    let shift = (index % 4) * 8;
    return icf_b[idx] >> shift & 0xFFu;
}

/// Returns the color of the background at the given index.
fn get_background(index: u32) -> u32 {
    let idx = index / width;
    return background[idx];
}

/// Makes a ARGB32 u32 value from the individual components.
///
/// alpha needs to be in the correct MSB. Red, green and blue must be in the LSB.
fn make_pixel(alpha: u32, red: u32, green: u32, blue: u32) -> u32 {
    return alpha | (red << 16) | (green << 8) | blue;
}

/// Applies ICF for a single color component.
fn apply_icf_component(color: u32, icf: u32) -> u32 {
    let c = bitcast<i32>(color);
    let i = bitcast<i32>(icf);
    let res = (((c - 16) * i) / 63) + 16;
    return bitcast<u32>(res);
}

/// Applies ICF on all RGB components of both pixels and overlays them.
fn apply_overlay(front: u32, back: u32, background: u32, icf_f: u32, icf_b: u32) -> u32 {
    let fa = front & 0xFF000000;
    let fr = apply_icf_component(front >> 16 & 0xFF, icf_f);
    let fg = apply_icf_component(front >> 8 & 0xFF, icf_f);
    let fb = apply_icf_component(front & 0xFF, icf_f);

    let ba = back & 0xFF000000;
    let br = apply_icf_component(back >> 16 & 0xFF, icf_b);
    let bg = apply_icf_component(back >> 8 & 0xFF, icf_b);
    let bb = apply_icf_component(back & 0xFF, icf_b);

    // TODO: benchmark moving the computations below.
    if fa == 0 && ba == 0 { // Front and back planes transparent: only show background.
        return background;
    } else if fa == 0 { // Front plane transparent: show back plane.
        return make_pixel(ba, br, bg, bb);
    } else { // Front plane visible: only show front plane.
        return make_pixel(fa, fr, fg, fb);
    }
}

/// Applies ICF on all RGB components of both pixels and mixes them.
fn apply_mix(front: u32, back: u32, background: u32, icf_f: u32, icf_b: u32) -> u32 {
    let fa = front & 0xFF000000;
    let fr = apply_icf_component(front >> 16 & 0xFF, icf_f);
    let fg = apply_icf_component(front >> 8 & 0xFF, icf_f);
    let fb = apply_icf_component(front & 0xFF, icf_f);

    let ba = back & 0xFF000000;
    let br = apply_icf_component(back >> 16 & 0xFF, icf_b);
    let bg = apply_icf_component(back >> 8 & 0xFF, icf_b);
    let bb = apply_icf_component(back & 0xFF, icf_b);

    // TODO: benchmark moving the computations below.
    if fa == 0 && ba == 0 { // Front and back planes transparent: only show background.
        return background;
    } else if fa == 0 { // Front plane transparent: show back plane.
        return make_pixel(ba, br, bg, bb);
    } else if ba == 0 { // Back plane transparent: only front plane.
        return make_pixel(fa, fr, fg, fb);
    } else { // Both planes visible: mix them.
        let a = 0xFF000000u;
        let r = clamp(fr + br - 16, 0, 255);
        let g = clamp(fg + bg - 16, 0, 255);
        let b = clamp(fb + bb - 16, 0, 255);
        return make_pixel(a, r, g, b);
    }
}

/// Implements the overlay algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn overlay_front_a(@builtin(global_invocation_id) id: vec3u) {
    let icf_f = get_icf_a(id.x);
    let icf_b = get_icf_b(id.x);

    screen[id.x] = apply_overlay(plane_a[id.x], plane_b[id.x], get_background(id.x), icf_f, icf_b);
}

/// Implements the overlay algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn overlay_front_b(@builtin(global_invocation_id) id: vec3u) {
    let icf_f = get_icf_b(id.x);
    let icf_b = get_icf_a(id.x);

    screen[id.x] = apply_overlay(plane_b[id.x], plane_a[id.x], get_background(id.x), icf_f, icf_b);
}

/// Implements the mixing algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn mix_front_a(@builtin(global_invocation_id) id: vec3u) {
    let icf_f = get_icf_a(id.x);
    let icf_b = get_icf_b(id.x);

    screen[id.x] = apply_mix(plane_a[id.x], plane_b[id.x], get_background(id.x), icf_f, icf_b);
}

/// Implements the mixing algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn mix_front_b(@builtin(global_invocation_id) id: vec3u) {
    let icf_f = get_icf_b(id.x);
    let icf_b = get_icf_a(id.x);

    screen[id.x] = apply_mix(plane_b[id.x], plane_a[id.x], get_background(id.x), icf_f, icf_b);
}
