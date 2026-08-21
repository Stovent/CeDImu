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

# On Windows, to use the corrosion_experimental_cbindgen function, this workaround is required:
# You need a local build of corrosion with the fix suggested in https://github.com/corrosion-rs/corrosion/issues/671
# When resolved, remove this and use the FetchContent_Declare above instead.
# find_package(Corrosion REQUIRED PATHS C:/Dev/corrosion EXCLUDE_FROM_ALL)

corrosion_import_crate(MANIFEST_PATH m68000/Cargo.toml)
# corrosion_experimental_cbindgen(
#     TARGET m68000_ffi
#     HEADER_NAME m68000_ffi.hpp
# )
target_include_directories(m68000_ffi INTERFACE m68000/include)
