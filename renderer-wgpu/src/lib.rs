//! Test implementation of a CD-I renderer in wgpu.

pub mod ffi;

use std::num::NonZeroU64;

use pollster::block_on;

#[cfg(feature = "renderdoc")]
use renderdoc::{RenderDoc, V141};

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
    pub front_plane_b: bool,
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

    pub matte_commands: &'a [u8],
    pub matte_number: &'a [u8],
    pub initial_icf_a: &'a [u8],
    pub initial_icf_b: &'a [u8],
}

/// The state of the wgpu renderer.
#[derive(Debug)]
pub struct WgpuRendererContext {
    /// Keep the adapter used to allow the caller to print info.
    pub adapter: Adapter,
    device: Device,
    queue: Queue,

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

    matte_commands_buffer: Buffer,
    matte_number_buffer: Buffer,
    width_buffer: Buffer,
    // size_buffer: Buffer,
    initial_icf_a_buffer: Buffer,
    initial_icf_b_buffer: Buffer,

    transfer_screen_buffer: Buffer,
    transfer_a_buffer: Buffer,
    transfer_b_buffer: Buffer,

    matte_transparency_bind_group: BindGroup,
    matte_pipeline: ComputePipeline,
    transparency_pipeline: ComputePipeline,

    overlay_mix_bind_group: BindGroup,
    overlay_front_a_pipeline: ComputePipeline,
    overlay_front_b_pipeline: ComputePipeline,
    mix_front_a_pipeline: ComputePipeline,
    mix_front_b_pipeline: ComputePipeline,
}

// static CDI_SHADER: &str = include_str!("cdi.wgsl");
static MATTE_TRANSPARENCY_SHADER: &str = include_str!("matte_transparency.wgsl");
static OVERLAY_MIX_SHADER: &str = include_str!("overlay_mix.wgsl");
/// The workgroup size, must match the value in the shader.
const WORKGROUP_SIZE: u32 = 64;
pub const MAX_HEIGHT: u64 = 560;
pub const MAX_HEIGHT_BYTE: u64 = MAX_HEIGHT * 4;
pub const MAX_FRAME_SIZE: u64 = 768 * MAX_HEIGHT;
pub const MAX_FRAME_SIZE_BYTE: u64 = MAX_FRAME_SIZE * 4;
pub const MAX_COMMANDS: u64 = MAX_HEIGHT * 8;
pub const MAX_COMMANDS_BYTE: u64 = MAX_COMMANDS * 4;
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
        limits.max_storage_buffers_per_shader_stage = 20;
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

        let matte_commands_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte commands buffer"),
            size: MAX_COMMANDS_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let matte_number_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte number buffer"),
            size: MAX_HEIGHT,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let width_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Width buffer"),
            size: SIZEOF_PIXEL, // Hopefully they have the same size on CPU and GPU.
            usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        // let size_buffer = device.create_buffer(&BufferDescriptor {
        //     label: Some("Size buffer"),
        //     size: SIZEOF_PIXEL, // Hopefully they have the same size on CPU and GPU.
        //     usage: BufferUsages::UNIFORM | BufferUsages::COPY_DST,
        //     mapped_at_creation: false,
        // });

        let initial_icf_a_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Initial ICF A buffer"),
            size: MAX_HEIGHT,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let initial_icf_b_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Initial ICF B buffer"),
            size: MAX_HEIGHT,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let matte_flag_0_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte flag 0 buffer"),
            size: MAX_FRAME_SIZE_BYTE,
            usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let matte_flag_1_buffer = device.create_buffer(&BufferDescriptor {
            label: Some("Matte flag 1 buffer"),
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

//         let last_icf_a_buffer = device.create_buffer(&BufferDescriptor {
//             label: Some("Last ICF A buffer"),
//             size: MAX_FRAME_SIZE_BYTE,
//             usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
//             mapped_at_creation: false,
//         });
//
//         let last_icf_b_buffer = device.create_buffer(&BufferDescriptor {
//             label: Some("Last ICF B buffer"),
//             size: MAX_FRAME_SIZE_BYTE,
//             usage: BufferUsages::STORAGE | BufferUsages::COPY_DST,
//             mapped_at_creation: false,
//         });

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

        // Matte transparency

        // Create the matte and transparency bind group layout
        let matte_transparency_bind_group_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: Some("Matte transparency bind group layout"),
            entries: &[
                BindGroupLayoutEntry {
                    binding: 0, // Plane A
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
                    binding: 1, // Plane B
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
                    binding: 2, // Transparency A
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
                    binding: 3, // Transparency B
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
                    binding: 4, // Mask color A
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
                    binding: 5, // Mask color B
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
                    binding: 6, // Transparent color A
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
                    binding: 7, // Transparent color B
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
                    binding: 8, // Matte commands
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
                    binding: 9, // Matte number
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
                    binding: 10, // Initial ICF A
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
                    binding: 11, // Initial ICF B
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
                    binding: 12, // Width
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Uniform,
                        has_dynamic_offset: false,
                        min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                    },
                    count: None,
                },
                // BindGroupLayoutEntry {
                //     binding: 13, // Size
                //     visibility: ShaderStages::COMPUTE,
                //     ty: BindingType::Buffer {
                //         ty: BufferBindingType::Uniform,
                //         has_dynamic_offset: false,
                //         min_binding_size: Some(NonZeroU64::new(SIZEOF_PIXEL).unwrap()),
                //     },
                //     count: None,
                // },
                BindGroupLayoutEntry {
                    binding: 20, // Matte flag 0
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 21, // Matte flag 1
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 22, // ICF A
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                },
                BindGroupLayoutEntry {
                    binding: 23, // ICF B
                    visibility: ShaderStages::COMPUTE,
                    ty: BindingType::Buffer {
                        ty: BufferBindingType::Storage {
                            read_only: false,
                        },
                        has_dynamic_offset: false,
                        min_binding_size: None,
                    },
                    count: None,
                },
                // BindGroupLayoutEntry {
                //     binding: 24, // Last ICF A
                //     visibility: ShaderStages::COMPUTE,
                //     ty: BindingType::Buffer {
                //         ty: BufferBindingType::Storage {
                //             read_only: false,
                //         },
                //         has_dynamic_offset: false,
                //         min_binding_size: None,
                //     },
                //     count: None,
                // },
                // BindGroupLayoutEntry {
                //     binding: 25, // Last ICF B
                //     visibility: ShaderStages::COMPUTE,
                //     ty: BindingType::Buffer {
                //         ty: BufferBindingType::Storage {
                //             read_only: false,
                //         },
                //         has_dynamic_offset: false,
                //         min_binding_size: None,
                //     },
                //     count: None,
                // },
            ],
        });

        // Create the bind group
        let matte_transparency_bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("Matte transparency bind group"),
            layout: &matte_transparency_bind_group_layout,
            entries: &[
                BindGroupEntry {
                    binding: 0, // Plane A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &plane_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 1, // Plane B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &plane_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 2, // Transparency A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparency_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 3, // Transparency B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparency_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 4, // Mask color A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &mask_color_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 5, // Mask color B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &mask_color_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 6, // Transparent color A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparent_color_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 7, // Transparent color B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &transparent_color_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 8, // Matte commands
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_commands_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 9, // Matte number
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_number_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 10, // Initial ICF A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &initial_icf_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 11, // Initial ICF B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &initial_icf_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 12, // Width
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &width_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                // BindGroupEntry {
                //     binding: 13, // Size
                //     resource: BindingResource::Buffer(BufferBinding {
                //         buffer: &size_buffer,
                //         offset: 0,
                //         size: None,
                //     }),
                // },
                BindGroupEntry {
                    binding: 20, // Matte flag 0
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_flag_0_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 21, // Matte flag 1
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &matte_flag_1_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 22, // ICF A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 23, // ICF B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                // BindGroupEntry {
                //     binding: 24, // Last ICF A
                //     resource: BindingResource::Buffer(BufferBinding {
                //         buffer: &last_icf_a_buffer,
                //         offset: 0,
                //         size: None,
                //     }),
                // },
                // BindGroupEntry {
                //     binding: 25, // Last ICF B
                //     resource: BindingResource::Buffer(BufferBinding {
                //         buffer: &last_icf_b_buffer,
                //         offset: 0,
                //         size: None,
                //     }),
                // },
            ],
        });

        let matte_transparency_pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("Matte transparency pipeline layout"),
            bind_group_layouts: &[&matte_transparency_bind_group_layout],
            immediate_size: 0,
        });

        let matte_transparency_shader_module = device.create_shader_module(ShaderModuleDescriptor {
            label: Some("Matte transparency shader"),
            source: ShaderSource::Wgsl(std::borrow::Cow::Borrowed(MATTE_TRANSPARENCY_SHADER)),
        });

        // Create the pipelines
        let matte_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Matte pipeline"),
            layout: Some(&matte_transparency_pipeline_layout),
            module: &matte_transparency_shader_module,
            entry_point: Some("matte"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let transparency_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Transparency pipeline"),
            layout: Some(&matte_transparency_pipeline_layout),
            module: &matte_transparency_shader_module,
            entry_point: Some("transparency"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        // Overlay/Mixing

        // Create the overlay/mix bind group layout
        let overlay_mix_bind_group_layout = device.create_bind_group_layout(&BindGroupLayoutDescriptor {
            label: Some("Overlay/Mix bind group layout"),
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
                            read_only: true,
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
                            read_only: true,
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
                    binding: 4, // ICF A
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
                    binding: 5, // ICF B
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
                    binding: 6, // Width
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
        let overlay_mix_bind_group = device.create_bind_group(&BindGroupDescriptor {
            label: Some("Overlay/Mix bind group"),
            layout: &overlay_mix_bind_group_layout,
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
                    binding: 4, // ICF A
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_a_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 5, // ICF B
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &icf_b_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
                BindGroupEntry {
                    binding: 6, // Width
                    resource: BindingResource::Buffer(BufferBinding {
                        buffer: &width_buffer,
                        offset: 0,
                        size: None,
                    }),
                },
            ],
        });

        // Create the overlay/mix pipeline layout
        let overlay_mix_pipeline_layout = device.create_pipeline_layout(&PipelineLayoutDescriptor {
            label: Some("Overlay/Mix pipeline layout"),
            bind_group_layouts: &[&overlay_mix_bind_group_layout],
            immediate_size: 0,
        });

        // Create the overlay/mixing module
        let overlay_mix_shader_module = device.create_shader_module(ShaderModuleDescriptor {
            label: Some("Overlay/Mix shader"),
            source: ShaderSource::Wgsl(std::borrow::Cow::Borrowed(OVERLAY_MIX_SHADER)),
        });

        let overlay_front_a_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Overlay front A pipeline"),
            layout: Some(&overlay_mix_pipeline_layout),
            module: &overlay_mix_shader_module,
            entry_point: Some("overlay_front_a"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let overlay_front_b_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Overlay front B pipeline"),
            layout: Some(&overlay_mix_pipeline_layout),
            module: &overlay_mix_shader_module,
            entry_point: Some("overlay_front_b"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let mix_front_a_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Mix front A pipeline"),
            layout: Some(&overlay_mix_pipeline_layout),
            module: &overlay_mix_shader_module,
            entry_point: Some("mix_front_a"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        let mix_front_b_pipeline = device.create_compute_pipeline(&ComputePipelineDescriptor {
            label: Some("Mix front B pipeline"),
            layout: Some(&overlay_mix_pipeline_layout),
            module: &overlay_mix_shader_module,
            entry_point: Some("mix_front_b"),
            compilation_options: SHADER_COMPILATION_OPTIONS.clone(),
            cache: None,
        });

        Self {
            adapter,
            device,
            queue,

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

            matte_commands_buffer,
            matte_number_buffer,
            width_buffer,
            // size_buffer,
            initial_icf_a_buffer,
            initial_icf_b_buffer,

            transfer_screen_buffer,
            transfer_a_buffer,
            transfer_b_buffer,

            matte_transparency_bind_group,
            matte_pipeline,
            transparency_pipeline,

            overlay_mix_bind_group,
            overlay_front_a_pipeline,
            overlay_front_b_pipeline,
            mix_front_a_pipeline,
            mix_front_b_pipeline,
        }
    }

    /// Renders a frame and waits until the result is copied to the screen.
    ///
    /// Note that this function does not check if the frame size matches the buffer sizes.
    pub fn render(&self, frame: &WgpuRendererFrame, buffers: &mut WgpuRendererBuffers) {
        #[cfg(feature = "renderdoc")]
        let mut renderdoc = {
            let mut renderdoc: RenderDoc<V141> = RenderDoc::new().unwrap_or_else(|param| panic!("{param:?}"));
            renderdoc.start_frame_capture(std::ptr::null(), std::ptr::null());
            renderdoc
        };

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

        self.queue.write_buffer(&self.matte_commands_buffer, 0, &buffers.matte_commands);
        self.queue.write_buffer(&self.matte_number_buffer, 0, &buffers.matte_number);

        self.queue.write_buffer(&self.width_buffer, 0, &frame.width.to_le_bytes());
        // self.queue.write_buffer(&self.size_buffer, 0, &frame.size().to_le_bytes());
        self.queue.write_buffer(&self.initial_icf_a_buffer, 0, &buffers.initial_icf_a);
        self.queue.write_buffer(&self.initial_icf_b_buffer, 0, &buffers.initial_icf_b);
        self.queue.submit([]); // Start copying the buffers before anything else to gain time.

        // The workgroup size being a divisor of the frame size is a key factor in the shader.
        let workgroup_count = frame_size.checked_div(WORKGROUP_SIZE).unwrap();

        // Create the command encoder
        let mut command_encoder = self.device.create_command_encoder(&CommandEncoderDescriptor {
            label: Some("Command encoder"),
        });

        // Wrap in a block so the compute pass is dropped and we can access the command encoder again.
        // Matte compute pass
        {
            let mut compute_pass = command_encoder.begin_compute_pass(&ComputePassDescriptor {
                label: Some("Matte compute pass"),
                timestamp_writes: None,
            });

            compute_pass.set_pipeline(&self.matte_pipeline);
            compute_pass.set_bind_group(0, Some(&self.matte_transparency_bind_group), &[]);

            // Finally dispath the workgroups
            compute_pass.dispatch_workgroups(workgroup_count, 1, 1);
        }

        // Transparency compute pass
        {
            let mut compute_pass = command_encoder.begin_compute_pass(&ComputePassDescriptor {
                label: Some("Transparency compute pass"),
                timestamp_writes: None,
            });

            compute_pass.set_pipeline(&self.transparency_pipeline);
            compute_pass.set_bind_group(0, Some(&self.matte_transparency_bind_group), &[]);

            // Finally dispath the workgroups
            compute_pass.dispatch_workgroups(workgroup_count, 1, 1);
        }

        // Overlay/mix compute pass
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
            compute_pass.set_bind_group(0, Some(&self.overlay_mix_bind_group), &[]);

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

        #[cfg(feature = "renderdoc")]
        {
            renderdoc.end_frame_capture(std::ptr::null(), std::ptr::null());
        }
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
