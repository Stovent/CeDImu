//! Compute shader for the matte and transparency parts of the CD-I.
//! The caller is responsible for calling the appropriate shader entry point.
//!
//! By default there is one pixel per index, or one value per line if stated height only,
//! and there is one element per value, unless specified with u8 which indicates each u32 has 4 u8 values for 4
//! different pixels.
//!
//! This shader is implemented as a data-parallel algorithm, where each function handles a single pixel.
//! Thus the caller must dispatch for exactly as many pixels as the screen.
//! There is no bounds checking in the kernels because the workgroupe size is 64,
//! which is a divisor of all the possible screen resolutions.

/// Source ARGB plane A.
@group(0) @binding(0) var<storage, read_write> plane_a: array<u32>;
/// Source ARGB plane B.
@group(0) @binding(1) var<storage, read_write> plane_b: array<u32>;

// TODO: use structs for all this, since this is one of each per line.
/// Source transparency control A (height only, u8).
@group(0) @binding(2) var<storage, read> transparency_a: array<u32>;
/// Source transparency control B (height only, u8).
@group(0) @binding(3) var<storage, read> transparency_b: array<u32>;
/// Source transparency mask color A (height only).
@group(0) @binding(4) var<storage, read> mask_color_a: array<u32>;
/// Source transparency mask color B (height only).
@group(0) @binding(5) var<storage, read> mask_color_b: array<u32>;
/// Source transparent color A (height only).
@group(0) @binding(6) var<storage, read> transparent_color_a: array<u32>;
/// Source transparent color B (height only).
@group(0) @binding(7) var<storage, read> transparent_color_b: array<u32>;
/// Source matte commands (height only) with 8 commands per line.
@group(0) @binding(8) var<storage, read> matte_commands: array<u32>;
/// Source matte number (height only, u8 of values 0 or 1).
@group(0) @binding(9) var<storage, read> matte_number: array<u32>;
/// Source initial ICF A (height only, u8).
@group(0) @binding(10) var<storage, read> initial_icf_a: array<u32>; // The ICF A value at the start of the line.
/// Source initial ICF B (height only, u8).
@group(0) @binding(11) var<storage, read> initial_icf_b: array<u32>; // The ICF B value at the start of the line.

/// The width of the image in pixels.
@group(0) @binding(12) var<uniform> width: u32;
// /// The size of the image in pixels.
// @group(0) @binding(13) var<uniform> size: u32;

const MAX_SCREEN_SIZE: u32 = 768 * 560;
/// Internal matte flags 0.
@group(0) @binding(20) var<storage, read_write> matte_flag_0: array<u32, MAX_SCREEN_SIZE>;
/// Internal matte flags 1.
@group(0) @binding(21) var<storage, read_write> matte_flag_1: array<u32, MAX_SCREEN_SIZE>;
/// Internal ICF A.
@group(0) @binding(22) var<storage, read_write> icf_a: array<u32, MAX_SCREEN_SIZE>;
/// Internal ICF B.
@group(0) @binding(23) var<storage, read_write> icf_b: array<u32, MAX_SCREEN_SIZE>;
// @group(0) @binding(24) var<storage, read_write> last_icf_a: u32;
// @group(0) @binding(25) var<storage, read_write> last_icf_b: u32;

///////////////////////////// Matte ////////////////////////////////////////////

/// Returns the pixel number on the line from the given global index.
fn get_line_position(idx: u32) -> u32 {
    return idx % width;
}

/// Returns the pixel number on the line from the given global index.
fn get_matte_number(index: u32) -> u32 {
    let i = index / width;
    let idx = i / 4; // sizeof(u32)
    let shift = (i % 4) * 8;
    return matte_number[idx] >> shift & 0xFF;
}

/// Returns the initial ICF A for the given pixel index.
fn get_initial_icf_a(index: u32) -> u32 {
    let i = index / width;
    let idx = i / 4; // sizeof(u32)
    let shift = (i % 4) * 8;
    return initial_icf_a[idx] >> shift & 0xFF;
}

/// Returns the initial ICF B for the given pixel index.
fn get_initial_icf_b(index: u32) -> u32 {
    let i = index / width;
    let idx = i / 4; // sizeof(u32)
    let shift = (i % 4) * 8;
    return initial_icf_b[idx] >> shift & 0xFF;
}

/// executes the matte command.
///
/// `matte_flag` is either 0 or 1 to specify the matte flag to modify, or > 1 to use the bit in the command.
///
/// Returns true when higher registers needs to be ignored, false to continue executing higher commands.
fn execute_matte_command(command: u32, matte_flag: u32, idx: u32) -> bool {
    var mf = matte_flag;
    if(matte_flag > 1) {
        mf = u32((command & 0x10000u) != 0);
    }

    let icf = (command >> 10) & 0x3Fu;
    let op = (command >> 20) & 0xFu;
    switch op {
        case 0x0: {
            return true;
        }
        case 0x4: {
            icf_a[idx] = icf;
        }
        case 0x6: {
            icf_b[idx] = icf;
        }
        case 0x8: {
            if mf == 0 {
                matte_flag_0[idx] = 0;
            } else {
                matte_flag_1[idx] = 0;
            }
        }
        case 0x9: {
            if mf == 0 {
                matte_flag_0[idx] = 1;
            } else {
                matte_flag_1[idx] = 1;
            }
        }
        case 0xC: {
            icf_a[idx] = icf;
            if mf == 0 {
                matte_flag_0[idx] = 0;
            } else {
                matte_flag_1[idx] = 0;
            }
        }
        case 0xD: {
            icf_a[idx] = icf;
            if mf == 0 {
                matte_flag_0[idx] = 1;
            } else {
                matte_flag_1[idx] = 1;
            }
        }
        case 0xE: {
            icf_b[idx] = icf;
            if mf == 0 {
                matte_flag_0[idx] = 0;
            } else {
                matte_flag_1[idx] = 0;
            }
        }
        case 0xF: {
            icf_b[idx] = icf;
            if mf == 0 {
                matte_flag_0[idx] = 1;
            } else {
                matte_flag_1[idx] = 1;
            }
        }
        default: {}
    }

    return false;
}

/// Implementation of one matte.
fn one_matte(idx: u32) {
    let matte_index = (idx / width) * 8;
    var min_next_pos = 0u; // This ensures we only have increasing positions.
    for(var i = 0u; i < 8; i += 1) {
        let command = matte_commands[matte_index + i];
        let position = (command & 0x3FF);

        if get_line_position(idx) < position || position < min_next_pos {
            break;
        }
        if execute_matte_command(command, 2, idx) {
            break;
        }
        min_next_pos = position + 1;
    }
}

/// Implementation of two mattes.
fn two_matte(idx: u32) {
    let matte_index_0 = (idx / width) * 8;
    var min_next_pos_0 = 0u; // This ensures we only have increasing positions.
    for(var i = 0u; i < 4; i += 1) {
        let command_0 = matte_commands[matte_index_0 + i];
        let position_0 = (command_0 & 0x3FF);

        if get_line_position(idx) < position_0 || position_0 < min_next_pos_0 {
            break;
        }
        if execute_matte_command(command_0, 0, idx) {
            break;
        }
        min_next_pos_0 = position_0 + 1;
    }

    let matte_index_1 = (idx / width) * 8 + 4;
    var min_next_pos_1 = 0u; // This ensures we only have increasing positions.
    for(var i = 4u; i < 8; i += 1) {
        let command_1 = matte_commands[matte_index_1 + i];
        let position_1 = (command_1 & 0x3FF);

        if get_line_position(idx) < position_1 || position_1 < min_next_pos_1 {
            break;
        }
        if execute_matte_command(command_1, 1, idx) {
            break;
        }
        min_next_pos_1 = position_1 + 1;
    }
}


@compute
@workgroup_size(64, 1, 1)
fn matte(@builtin(global_invocation_id) id: vec3u) {
    icf_a[id.x] = get_initial_icf_a(id.x);
    icf_b[id.x] = get_initial_icf_b(id.x);
    if get_matte_number(id.x) == 0 {
        one_matte(id.x);
    } else {
        two_matte(id.x);
    }
    // TODO: how to handle the case where I need the one from the previous line?
    // TODO: read latest icf back to CPU for the next frame?
    // if id.x == (size - 1) {
    //     initial_icf_a = icf_a[id.x];
    //     initial_icf_b = icf_b[id.x];
    // }
}

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

/// Masks the given color to the actually used bytes (V.5.7.2.2).
fn clut_color_key(color: u32) -> u32 {
    return color & 0x00FCFCFC;
}

/// Computes the transparency for both planes.
fn handle_transparency(pixel: u32, transparent: u32, mask_color: u32, transparent_color: u32, index: u32) -> u32 {
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
            hidden = ((pixel & 0xFF000000) != 0) != boolean;
        }
        case 3: { // Matte Flag 0.
            hidden = matte_flag_0[index] == u32(boolean);
        }
        case 4: { // Matte Flag 1.
            hidden = matte_flag_1[index] == u32(boolean);
        }
        case 5: { // Matte Flag 0 or Color Key.
            hidden = (matte_flag_0[index] == u32(boolean)) || (color_key == boolean);
        }
        case 6: { // Matte Flag 1 or Color Key.
            hidden = (matte_flag_1[index] == u32(boolean)) || (color_key == boolean);
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
                                        get_transparent_color_a(id.x), id.x);
    plane_b[id.x] = handle_transparency(plane_b[id.x], get_transparent_b(id.x), get_mask_color_b(id.x),
                                        get_transparent_color_b(id.x), id.x);
}
