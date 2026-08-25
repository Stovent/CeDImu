cmake_minimum_required(VERSION 3.25)

project(renderer-wgpu)

include(FetchContent)
FetchContent_Declare(
    Corrosion
    GIT_REPOSITORY https://github.com/corrosion-rs/corrosion.git
    # v0.6 will be updated to point to the latest patch version.
    # Use v0.6.<patch_version> or the commit hash to prevent such auto updates.
    GIT_TAG v0.6
)
# Set any global configuration variables such as `Rust_TOOLCHAIN` before this line!
FetchContent_MakeAvailable(Corrosion)

corrosion_import_crate(MANIFEST_PATH ./Cargo.toml)
corrosion_experimental_cbindgen(
    TARGET renderer_wgpu
    HEADER_NAME RendererWGPU_ffi.hpp
)
if(APPLE)
    target_link_options(renderer_wgpu INTERFACE -framework Metal)
endif()
