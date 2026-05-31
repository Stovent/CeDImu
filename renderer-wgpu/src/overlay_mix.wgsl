//! Compute shader for the overlay/mixing part of the CD-I.
//! The caller is responsible for calling the appropriate shader entry point.
//!
//! By default there is one pixel per index, or one value per line if stated height only.
//!
//! This shader is implemented as a data-parallel algorithm, where each function handles a single pixel.
//! Thus the caller must dispatch for exactly as many pixels as the screen.
//! There is no bounds checking in the kernels because the workgroupe size is 64,
//! which is a divisor of all the possible screen resolutions.

/// Destination buffer.
@group(0) @binding(0) var<storage, read_write> screen: array<u32>;
/// Source ARGB plane A.
@group(0) @binding(1) var<storage, read> plane_a: array<u32>;
/// Source ARGB plane B.
@group(0) @binding(2) var<storage, read> plane_b: array<u32>;
/// Source background ARGB (height only).
@group(0) @binding(3) var<storage, read> background: array<u32>;
/// Source ICF A values.
@group(0) @binding(4) var<storage, read> icf_a: array<u32>;
/// Source ICF B values.
@group(0) @binding(5) var<storage, read> icf_b: array<u32>;
/// The width of the image in pixels.
@group(0) @binding(6) var<uniform> width: u32;

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
    let fa = front & 0xFF000000u;
    let ba = back & 0xFF000000u;

    if fa == 0 && ba == 0 { // Front and back planes transparent: only show background.
        return background;
    } else if fa == 0 { // Front plane transparent: show back plane.
        let br = apply_icf_component(back >> 16 & 0xFF, icf_b);
        let bg = apply_icf_component(back >> 8 & 0xFF, icf_b);
        let bb = apply_icf_component(back & 0xFF, icf_b);
        return make_pixel(ba, br, bg, bb);
    } else { // Front plane visible: only show front plane.
        let fr = apply_icf_component(front >> 16 & 0xFF, icf_f);
        let fg = apply_icf_component(front >> 8 & 0xFF, icf_f);
        let fb = apply_icf_component(front & 0xFF, icf_f);
        return make_pixel(fa, fr, fg, fb);
    }
}

/// Applies ICF on all RGB components of both pixels and mixes them.
fn apply_mix(front: u32, back: u32, background: u32, icf_f: u32, icf_b: u32) -> u32 {
    let fa = front & 0xFF000000u;
    let fr = apply_icf_component(front >> 16 & 0xFF, icf_f);
    let fg = apply_icf_component(front >> 8 & 0xFF, icf_f);
    let fb = apply_icf_component(front & 0xFF, icf_f);

    let ba = back & 0xFF000000u;
    let br = apply_icf_component(back >> 16 & 0xFF, icf_b);
    let bg = apply_icf_component(back >> 8 & 0xFF, icf_b);
    let bb = apply_icf_component(back & 0xFF, icf_b);

    if fa == 0 && ba == 0 { // Front and back planes transparent: only background.
        return background;
    } else if fa == 0 { // Front plane transparent: only back plane.
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
    screen[id.x] = apply_overlay(plane_a[id.x], plane_b[id.x], get_background(id.x), icf_a[id.x], icf_b[id.x]);
}

/// Implements the overlay algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn overlay_front_b(@builtin(global_invocation_id) id: vec3u) {
    screen[id.x] = apply_overlay(plane_b[id.x], plane_a[id.x], get_background(id.x), icf_b[id.x], icf_a[id.x]);
}

/// Implements the mixing algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn mix_front_a(@builtin(global_invocation_id) id: vec3u) {
    screen[id.x] = apply_mix(plane_a[id.x], plane_b[id.x], get_background(id.x), icf_a[id.x], icf_b[id.x]);
}

/// Implements the mixing algorithm as a 1D flat array.
@compute
@workgroup_size(64, 1, 1) // Use 1D anyway.
fn mix_front_b(@builtin(global_invocation_id) id: vec3u) {
    screen[id.x] = apply_mix(plane_b[id.x], plane_a[id.x], get_background(id.x), icf_b[id.x], icf_a[id.x]);
}
