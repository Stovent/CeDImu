/** \file SIMD.hpp
 * \brief SIMD types and functions for RendererSIMD and VideoSIMD.
 */

#ifndef CDI_VIDEO_SIMD_HPP
#define CDI_VIDEO_SIMD_HPP

#include "Pixel.hpp"

#if __has_include(<simd>)
// #   include <simd>
#   warning not yet implemented with <simd> header, using <experimental/simd> instead
#endif
// #elif __has_include(<experimental/simd>)
#   include <experimental/simd>
// #else
// #    error Missing <simd> or <experimental/simd> headers
// #endif

namespace Video
{

#if __has_include(<simd>)
#   warning not yet implemented with <simd> header, using <experimental/simd> instead
#endif
// #elif __has_include(<experimental/simd>)

namespace stdx = std::experimental;
using SIMDNativePixel = stdx::native_simd<Pixel::ARGB32>;
using SIMDNativePixelMask = SIMDNativePixel::mask_type;
using SIMDNativePixelSigned = stdx::native_simd<std::make_signed_t<Pixel::ARGB32>>;
template<size_t WIDTH>
using SIMDFixedPixelSigned = stdx::fixed_size_simd<std::make_signed_t<Pixel::ARGB32>, WIDTH>;
using SIMDNativeU8 = stdx::native_simd<uint8_t>;
using SIMDFixedS16 = stdx::rebind_simd_t<int16_t, SIMDNativeU8>;

// #else
// #   error Missing <simd> or <experimental/simd> headers
// #endif

inline constexpr size_t SIMD_SIZE = SIMDNativePixel::size();

/** \brief Gives the number of elements that cannot be processed by a native-width SIMD.
 * \tparam WIDTH the width in pixels.
 */
template<size_t WIDTH>
struct SIMDReminder : std::integral_constant<size_t, WIDTH % SIMD_SIZE>
{};

} // namespace Video

#endif // CDI_VIDEO_SIMD_HPP
