//! Test implementation of a CD-I renderer in wgpu.

pub mod ffi;

use std::num::NonZeroU64;

use pollster::block_on;

use wgpu::Adapter;
use wgpu::BackendOptions;
use wgpu::Backends;
use wgpu::BindGroup;
use wgpu::BindGroupDescriptor;
use wgpu::BindGroupEntry;
use wgpu::BindGroupLayoutDescriptor;
use wgpu::BindGroupLayoutEntry;
use wgpu::BindingResource;
use wgpu::BindingType;
use wgpu::Buffer;
use wgpu::BufferBinding;
use wgpu::BufferBindingType;
use wgpu::BufferDescriptor;
use wgpu::BufferUsages;
use wgpu::CommandEncoderDescriptor;
use wgpu::ComputePassDescriptor;
use wgpu::ComputePipeline;
use wgpu::ComputePipelineDescriptor;
use wgpu::Device;
use wgpu::DeviceDescriptor;
use wgpu::Instance;
use wgpu::InstanceDescriptor;
use wgpu::InstanceFlags;
use wgpu::MapMode;
use wgpu::MemoryBudgetThresholds;
use wgpu::MemoryHints;
use wgpu::PipelineCompilationOptions;
use wgpu::PipelineLayoutDescriptor;
use wgpu::PowerPreference;
use wgpu::Queue;
use wgpu::RequestAdapterOptions;
use wgpu::ShaderModuleDescriptor;
use wgpu::ShaderSource;
use wgpu::ShaderStages;

/// The parameters needed to render a frame.
#[derive(Debug)]
#[repr(C)]
pub struct WgpuRendererFrame {
    /// Width in pixels of the frame to render.
    pub width: u32,
    /// Height in pixels of the frame to render.
    pub height: u32,
    /// True when mixing, false when overlaying.
    pub mix: bool,
    /// True when plane B in front of plane A.
    pub front_plane_b: bool
}

impl WgpuRendererFrame {
    /// Returns the number of pixels in the frame.
    #[inline]
    pub const fn size(&self) -> u32 {
        self.width * self.height
    }
}

/// Holds the pointers to the input and output buffers.
#[derive(Debug)]
pub struct WgpuRendererBuffers<'a> {
    pub screen: &'a mut [u8],
    pub plane_a: &'a mut [u8],
    pub plane_b: &'a mut [u8],
    pub background: &'a [u8],

    pub transparency_a: &'a [u8],
    pub transparency_b: &'a [u8],
    pub mask_color_a: &'a [u8],
    pub mask_color_b: &'a [u8],
    pub transparent_color_a: &'a [u8],
    pub transparent_color_b: &'a [u8],
    pub matte_flag_a: &'a [u8],
    pub matte_flag_b: &'a [u8],
    pub icf_a: &'a [u8],
    pub icf_b: &'a [u8],
}

/// The state of the wgpu renderer.
#[derive(Debug)]
pub struct WgpuRendererContext {
    /// Keep the adapter used to allow the caller to print info.
    pub adapter: Adapter,
    device: Device,
    queue: Queue,
    bind_group: BindGroup,

    screen_buffer: Buffer,
    plane_a_buffer: Buffer,
    plane_b_buffer: Buffer,
    background_buffer: Buffer,

    transparency_a_buffer: Buffer,
    transparency_b_buffer: Buffer,
    mask_color_a_buffer: Buffer,
    mask_color_b_buffer: Buffer,
    transparent_color_a_buffer: Buffer,
    transparent_color_b_buffer: Buffer,
    matte_flag_a_buffer: Buffer,
    matte_flag_b_buffer: Buffer,
    icf_a_buffer: Buffer,
    icf_b_buffer: Buffer,
    width_buffer: Buffer,

    transfer_screen_buffer: Buffer,
    transfer_a_buffer: Buffer,
    transfer_b_buffer: Buffer,

    transparency_pipeline: ComputePipeline,
    overlay_front_a_pipeline: ComputePipeline,
    overlay_front_b_pipeline: ComputePipeline,
    mix_front_a_pipeline: ComputePipeline,
    mix_front_b_pipeline: ComputePipeline,
}

/// The shader itself.
static CDI_SHADER: &str = include_str!("cdi.wgsl");
/// The workgroup size, must match the value in the shader.
const WORKGROUP_SIZE: u32 = 64;
pub const MAX_HEIGHT: u64 = 560;
pub const MAX_HEIGHT_BYTE: u64 = MAX_HEIGHT * 4;
pub const MAX_FRAME_SIZE: u64 = 768 * MAX_HEIGHT;
pub const MAX_FRAME_SIZE_BYTE: u64 = MAX_FRAME_SIZE * 4;
pub const SIZEOF_PIXEL: u64 = size_of::<u32>() as u64;

static SHADER_COMPILATION_OPTIONS: PipelineCompilationOptions = PipelineCompilationOptions {
    constants: &[],
    zero_initialize_workgroup_memory: false,
};

impl WgpuRendererContext {
    pub fn new() -> Self {
        // Create instance
        let instance = Instance::new(&InstanceDescriptor {
            backends: Backends::PRIMARY,
            flags: InstanceFlags::empty(),
            memory_budget_thresholds: MemoryBudgetThresholds::default(),
            backend_options: BackendOptions::default(),
        });

        // Get adapter
        let adapter = block_on(instance.request_adapter(&RequestAdapterOptions {
                power_preference: PowerPreference::HighPerformance,
                force_fallback_adapter: false,
                compatible_surface: None,
            }))
            .unwrap_or_else(|e| panic!("Failed to get adapter: {e}"));

        let adapter_info = adapter.get_info();
        println!("[WgpuRendererContext] Using adapter {} ({:?}, {})", adapter_info.name, adapter_info.device_type, adapter_info.backend);

        let mut limits = adapter.limits();
        limits.max_storage_buffers_per_shader_stage = 14;
        // Get device and command queue
        let (device, queue) = block_on(adapter.request_device(&DeviceDescriptor {
                label: Some("Device"),
                memory_hints: MemoryHints::Performance,
                required_limits: limits,
                ..Default::default()
            }))
            .unwrap_or_else(|e| panic!("Failed tp request device: {e}"));

        // Create all the buffers
        let screen_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Screen buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });

        let plane_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Plane A buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST | BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });

        let plane_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Plane B buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST | BufferUsages::COPY_SRC,
            mapped_at_creation: false,
        });

        let background_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Background buffer"),
            size: MAX_HEIGHT_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let transparency_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transparency A buffer"),
            size: MAX_HEIGHT,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let transparency_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transparency B buffer"),
            size: MAX_HEIGHT,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let mask_color_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Mask color A buffer"),
            size: MAX_HEIGHT_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let mask_color_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Mask color B buffer"),
            size: MAX_HEIGHT_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let transparent_color_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transparent color A buffer"),
            size: MAX_HEIGHT_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let transparent_color_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transparent color B buffer"),
            size: MAX_HEIGHT_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let matte_flag_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte flag A buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let matte_flag_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte flag B buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let icf_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("ICF A buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let icf_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("ICF B buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let width_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Width buffer"),
            size: SIZEOF_PIXEL, // Hopefully they have the same size on CPU and GPU.
            usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let transfer_screen_buffer: Buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transfer screen buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::COPY_DST | BufferUsages::MAP_READ,
            mapped_at_creation: false,
        });

        let transfer_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transfer A buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::COPY_DST | BufferUsages::MAP_READ,
            mapped_at_creation: false,
        });

        let transfer_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Transfer B buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::COPY_DST | BufferUsages::MAP_READ,
            mapped_at_creation: false,
        });

        // Create our bind group layout
        let bind_group_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: Some("BindGroupLayout"),
            entries: &[
                BindGroupLayoutEntry {
                    binding: 0, // Screen
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 1, // Plane A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 2, // Plane B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 3, // Background
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 4, // Transparency A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 5, // Transparency B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 6, // Mask color A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 7, // Mask color B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 8, // Transparent color A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 9, // Transparent color B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 10, // Matte flag A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 11, // Matte flag B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 12, // ICF A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 13, // ICF B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: true,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 14, // Width
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
            ],
        });

        // Create the bind group
        let bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("BindGroup"),
            layout: &bind_group_layout,
            entries: &[
                BindGroupEntry {
                    binding: 0, // Screen
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &screen_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 1, // Plane A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &plane_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 2, // Plane B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &plane_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 3, // Background
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &background_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 4, // Transparency A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparency_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 5, // Transparency B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparency_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 6, // Mask color A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &mask_color_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 7, // Mask color B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &mask_color_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 8, // Transparent color A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparent_color_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 9, // Transparent color B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparent_color_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 10, // Matte flag A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_flag_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 11, // Matte flag B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_flag_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 12, // ICF A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 13, // ICF B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 14, // Width
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &width_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
            ],
        });

        // Create the pipeline layout
        let pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("PipelineLayout"),
            bind_group_layouts: &[&bind_group_layout],
            immediate_size: 0,
        });

        // Create the shader module
        let shader_module = device.create_shader_module(ShaderModuleDescriptor {
            label: Some("CD-I shader"),
            source: ShaderSource::Wgsl(std::borrow::Cow::Borrowed(CDI_SHADER)),
        });

        // Create the pipelines
        let transparency_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Transparency pipeline"),
            layout: Some(&pipeline_layout),
            module: &shader_module,
            entry_point: Some("transparency"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let overlay_front_a_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Overlay front A pipeline"),
            layout: Some(&pipeline_layout),
            module: &shader_module,
            entry_point: Some("overlay_front_a"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let overlay_front_b_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Overlay front B pipeline"),
            layout: Some(&pipeline_layout),
            module: &shader_module,
            entry_point: Some("overlay_front_b"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let mix_front_a_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Mix front A pipeline"),
            layout: Some(&pipeline_layout),
            module: &shader_module,
            entry_point: Some("mix_front_a"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let mix_front_b_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Mix front B pipeline"),
            layout: Some(&pipeline_layout),
            module: &shader_module,
            entry_point: Some("mix_front_b"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        Self {
            adapter,
            device,
            queue,
            bind_group,

            screen_buffer,
            plane_a_buffer,
            plane_b_buffer,
            background_buffer,

            transparency_a_buffer,
            transparency_b_buffer,
            mask_color_a_buffer,
            mask_color_b_buffer,
            transparent_color_a_buffer,
            transparent_color_b_buffer,
            matte_flag_a_buffer,
            matte_flag_b_buffer,
            icf_a_buffer,
            icf_b_buffer,
            width_buffer,

            transfer_screen_buffer,
            transfer_a_buffer,
            transfer_b_buffer,

            transparency_pipeline,
            overlay_front_a_pipeline,
            overlay_front_b_pipeline,
            mix_front_a_pipeline,
            mix_front_b_pipeline,
        }
    }

    /// Renders a frame and waits until the result is copied to the screen.
    ///
    /// Note that this function does not check if the frame size matches the buffer sizes.
    #[allow(clippy::too_many_arguments)]
    pub fn render(&self, frame: &WgpuRendererFrame, buffers: &mut WgpuRendererBuffers) {
        // let start = std::time::Instant::now();

        // Multiply all by 4 below because the actual buffers are pixels, not u8s.
        let frame_size = frame.size();
        let frame_size_byte = frame_size * 4;
        let range = ..frame_size_byte as usize;
        let range64 = ..frame_size_byte as u64;

        self.queue.write_buffer(&self.plane_a_buffer, 0, &buffers.plane_a);
        self.queue.write_buffer(&self.plane_b_buffer, 0, &buffers.plane_b);
        self.queue.write_buffer(&self.background_buffer, 0, &buffers.background);

        self.queue.write_buffer(&self.transparency_a_buffer, 0, &buffers.transparency_a);
        self.queue.write_buffer(&self.transparency_b_buffer, 0, &buffers.transparency_b);
        self.queue.write_buffer(&self.mask_color_a_buffer, 0, &buffers.mask_color_a);
        self.queue.write_buffer(&self.mask_color_b_buffer, 0, &buffers.mask_color_b);
        self.queue.write_buffer(&self.transparent_color_a_buffer, 0, &buffers.transparent_color_a);
        self.queue.write_buffer(&self.transparent_color_b_buffer, 0, &buffers.transparent_color_b);
        self.queue.write_buffer(&self.matte_flag_a_buffer, 0, &buffers.matte_flag_a);
        self.queue.write_buffer(&self.matte_flag_b_buffer, 0, &buffers.matte_flag_b);
        self.queue.write_buffer(&self.icf_a_buffer, 0, &buffers.icf_a);
        self.queue.write_buffer(&self.icf_b_buffer, 0, &buffers.icf_b);

        self.queue.write_buffer(&self.width_buffer, 0, &frame.width.to_le_bytes());
        self.queue.submit([]); // Start copying the buffers before anything else to gain time.

         // The workgroup size being a divisor of the frame size is a key factor in the shader.
        let workgroup_count = frame_size.checked_div(WORKGROUP_SIZE).unwrap();

        // Create the command encoder
        let mut command_encoder = self.device.create_command_encoder(&CommandEncoderDescriptor {
            label: Some("CommandEncoder"),
        });

        // Wrap in a block so the compute pass is dropped and we can access the command encoder again
        // Transparency compute pass
        {
            let mut compute_pass = command_encoder.begin_compute_pass(&ComputePassDescriptor {
                label: Some("Transparency compute pass"),
                timestamp_writes: None,
            });

            compute_pass.set_pipeline(&self.transparency_pipeline);
            compute_pass.set_bind_group(0, Some(&self.bind_group), &[]);

            // Finally dispath the workgroups
            compute_pass.dispatch_workgroups(workgroup_count, 1, 1);
        }

        // Overlay/mixing compute pass
        {
            let mut compute_pass = command_encoder.begin_compute_pass(&ComputePassDescriptor {
                label: Some("Overlay/Mix compute pass"),
                timestamp_writes: None,
            });

            if frame.mix {
                if frame.front_plane_b {
                    compute_pass.set_pipeline(&self.mix_front_b_pipeline);
                } else {
                    compute_pass.set_pipeline(&self.mix_front_a_pipeline);
                }
            } else {
                if frame.front_plane_b {
                    compute_pass.set_pipeline(&self.overlay_front_b_pipeline);
                } else {
                    compute_pass.set_pipeline(&self.overlay_front_a_pipeline);
                }
            }
            compute_pass.set_bind_group(0, Some(&self.bind_group), &[]);

            // Finally dispath the workgroups
            compute_pass.dispatch_workgroups(workgroup_count, 1, 1);
        }

        // Copy results to the transfer buffer
        let read_size = Some(frame_size_byte.into());
        command_encoder.copy_buffer_to_buffer(&self.screen_buffer, 0, &self.transfer_screen_buffer, 0, read_size);
        command_encoder.copy_buffer_to_buffer(&self.plane_a_buffer, 0, &self.transfer_a_buffer, 0, read_size);
        command_encoder.copy_buffer_to_buffer(&self.plane_b_buffer, 0, &self.transfer_b_buffer, 0, read_size);

        // End of commands
        let command_buffer = command_encoder.finish();

        // Finally actually execute everything
        self.queue.submit([command_buffer]);

        // Transfer back to the CPU
        self.transfer_screen_buffer.map_async(MapMode::Read, .., |_| {});
        self.transfer_a_buffer.map_async(MapMode::Read, .., |_| {});
        self.transfer_b_buffer.map_async(MapMode::Read, .., |_| {});

        // Wait for transfer to complete
        // device.poll(wgpu::PollType::Poll).unwrap();
        self.device.poll(wgpu::PollType::wait_indefinitely()).unwrap();

        buffers.screen[range].copy_from_slice(&self.transfer_screen_buffer.slice(range64).get_mapped_range());
        buffers.plane_a[range].copy_from_slice(&self.transfer_a_buffer.slice(range64).get_mapped_range());
        buffers.plane_b[range].copy_from_slice(&self.transfer_b_buffer.slice(range64).get_mapped_range());
        self.transfer_screen_buffer.unmap();
        self.transfer_a_buffer.unmap();
        self.transfer_b_buffer.unmap();

        // Statistics
        // let duration = start.elapsed();
        // println!("Frame done in {duration:?}");
    }

    // /// Same as render except reading the screen buffer back is done asynchronously, and will finish after this function returns.
    // pub fn render_async(&self, frame: &WgpuRendererFrame, screen: &mut [u8], plane_a: &[u8], plane_b: &[u8], background: &[u8], icf_a: &[u8], icf_b: &[u8]) {
    // }
}

impl Default for WgpuRendererContext {
    fn default() -> Self {
        Self::new()
    }
}
