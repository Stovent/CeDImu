//! The FFI part of the renderer, so it can be accessed by the C++ code.
//!
//! To generate the header: from the root dir of CeDImu, call
//! `cbindgen --config ./renderer-wgpu/cbindgen.toml --output ./src/CDI/Video/RendererWGPU_ffi.hpp ./renderer-wgpu/`
//!
//! From inside renderer-wgpu: `cbindgen --config ./cbindgen.toml --output ../src/CDI/Video/RendererWGPU_ffi.hpp`
//!
//! To build the library: from the renderer-wgpu directory, call `cargo [+stable-x86_64-pc-windows-gnu] build -r`
//!
//! TODO: use cbindgen in the build.rs file?

use core::slice::{from_raw_parts, from_raw_parts_mut};

use crate::{WgpuRendererBuffers, WgpuRendererContext, WgpuRendererFrame};

/// Allocates a new Wgpu renderer.
#[unsafe(no_mangle)]
pub extern "C" fn renderer_wgpu_new() -> *mut WgpuRendererContext {
    Box::into_raw(Box::new(WgpuRendererContext::new()))
}

/// Frees the memory of a renderer allocated with [renderer_wgpu_new].
///
/// # Safety
/// Make sure the pointer is valid and not already freed.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn renderer_wgpu_delete(renderer: *mut WgpuRendererContext) {
    unsafe {
        std::mem::drop(Box::from_raw(renderer));
    }
}

/// Holds the pointers to all the input and output buffers.
#[derive(Debug, Copy, Clone)]
#[repr(C)]
pub struct WgpuRendererBuffersFfi {
    pub screen: *mut u8,
    pub plane_a: *mut u8,
    pub plane_b: *mut u8,
    pub background: *const u8,

    pub transparency_a: *const u8,
    pub transparency_b: *const u8,
    pub mask_color_a: *const u8,
    pub mask_color_b: *const u8,
    pub transparent_color_a: *const u8,
    pub transparent_color_b: *const u8,

    pub matte_commands: *const u8,
    pub matte_number: *const u8,
    pub initial_icf_a: *const u8,
    pub initial_icf_b: *const u8,
}

/// Renders a frame using wgpu.
///
/// # Safety
/// The caller have the responsibility that the pointers are valid and that they can be read from for as much pixels as
/// a full-sized screen.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn renderer_wgpu_render(renderer: *const WgpuRendererContext, frame: &WgpuRendererFrame, buffers: &WgpuRendererBuffersFfi) {
    unsafe {
        let size_pixels = frame.size() as usize;
        let size_bytes = size_pixels * 4; // 4 bytes per pixel.
        let height = frame.height as usize;
        let height_bytes = height as usize * 4; // 4 bytes per pixel.

        let mut buf = WgpuRendererBuffers {
            screen: from_raw_parts_mut(buffers.screen, size_bytes),
            plane_a: from_raw_parts_mut(buffers.plane_a, size_bytes),
            plane_b: from_raw_parts_mut(buffers.plane_b, size_bytes),
            background: from_raw_parts(buffers.background, height_bytes),

            transparency_a: from_raw_parts(buffers.transparency_a, height),
            transparency_b: from_raw_parts(buffers.transparency_b, height),
            mask_color_a: from_raw_parts(buffers.mask_color_a, height_bytes),
            mask_color_b: from_raw_parts(buffers.mask_color_b, height_bytes),
            transparent_color_a: from_raw_parts(buffers.transparent_color_a, height_bytes),
            transparent_color_b: from_raw_parts(buffers.transparent_color_b, height_bytes),
            matte_commands: from_raw_parts(buffers.matte_commands, height_bytes * 8),
            matte_number: from_raw_parts(buffers.matte_number, height),
            initial_icf_a: from_raw_parts(buffers.initial_icf_a, height),
            initial_icf_b: from_raw_parts(buffers.initial_icf_b, height),
        };
        (*renderer).render(&frame, &mut buf);
    }
}
