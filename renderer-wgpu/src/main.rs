//! Test binary that tests and benchmarks the CD-I renderer.

use std::time::Duration;

use renderer_wgpu::MAX_FRAME_SIZE_BYTE;
use renderer_wgpu::MAX_HEIGHT;
use renderer_wgpu::MAX_HEIGHT_BYTE;
use renderer_wgpu::WgpuRendererBuffers;
use renderer_wgpu::WgpuRendererContext;
use renderer_wgpu::WgpuRendererFrame;

fn main() {
    let setup_start = std::time::Instant::now();

    let ctx = WgpuRendererContext::new();

    let setup_duration = setup_start.elapsed();

    let mut screen = vec![0; MAX_FRAME_SIZE_BYTE as usize];
    let mut plane_a = vec![0; MAX_FRAME_SIZE_BYTE as usize];
    let mut plane_b = vec![0; MAX_FRAME_SIZE_BYTE as usize];
    let background = vec![0; MAX_HEIGHT_BYTE as usize];

    let transparency_a = vec![0; MAX_HEIGHT as usize];
    let transparency_b = vec![0; MAX_HEIGHT as usize];
    let mask_color_a = vec![0; MAX_HEIGHT as usize];
    let mask_color_b = vec![0; MAX_HEIGHT as usize];
    let transparent_color_a = vec![0; MAX_HEIGHT as usize];
    let transparent_color_b = vec![0; MAX_HEIGHT as usize];
    let matte_commands = vec![0; MAX_HEIGHT_BYTE as usize * 8];
    let matte_number = vec![0; MAX_HEIGHT as usize];
    let initial_icf_a = vec![0; MAX_HEIGHT as usize];
    let initial_icf_b = vec![0; MAX_HEIGHT as usize];

    let mut min = Duration::MAX;
    let mut max = Duration::default();
    let mut cumulative_duration = Duration::default();
    const COUNT: usize = 2000;
    for _ in 0..COUNT {
        let start = std::time::Instant::now();

        let frame = WgpuRendererFrame {
            width: 768,
            height: 280,
            mix: true,
            front_plane_b: false,
        };

        let mut buffers = WgpuRendererBuffers {
            screen: &mut screen,
            plane_a: &mut plane_a,
            plane_b: &mut plane_b,
            background: &background,

            transparency_a: &transparency_a,
            transparency_b: &transparency_b,
            mask_color_a: &mask_color_a,
            mask_color_b: &mask_color_b,
            transparent_color_a: &transparent_color_a,
            transparent_color_b: &transparent_color_b,
            matte_commands: &matte_commands,
            matte_number: &matte_number,
            initial_icf_a: &initial_icf_a,
            initial_icf_b: &initial_icf_b,
        };

        ctx.render(&frame, &mut buffers);

        // Statistics
        let duration = start.elapsed();
        cumulative_duration += duration;
        min = min.min(duration);
        max = max.max(duration);
    }

    println!("Setup done in {setup_duration:?}");
    println!("Compute done in min: {min:?}, avg {:?}, max {max:?}", cumulative_duration / COUNT as u32);
}
