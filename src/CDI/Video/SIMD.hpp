/** \file SIMD.hpp
 * \brief SIMD types and functions for RendererSIMD and VideoSIMD.
 */

#ifndef CDI_VIDEO_SIMD_HPP
#define CDI_VIDEO_SIMD_HPP

#include "Pixel.hpp"

#include <experimental/simd>
namespace stdx = std::experimental;
#if __cpp_lib_simd
#warning "SIMD is no longer experimental"
#endif

namespace Video
{

using SIMDNativePixel = stdx::native_simd<Pixel::ARGB32>;
using SIMDNativePixelSigned = stdx::native_simd<std::make_signed_t<Pixel::ARGB32>>;
template<size_t WIDTH>
using SIMDFixedPixelSigned = stdx::fixed_size_simd<std::make_signed_t<Pixel::ARGB32>, WIDTH>;
using SIMDNativeU8 = stdx::native_simd<uint8_t>;
using SIMDNativeS16 = stdx::native_simd<int16_t>;
using SIMDFixedS16 = stdx::fixed_size_simd<int16_t, SIMDNativeS16::size() * sizeof(SIMDNativeS16::value_type)>;

inline constexpr size_t SIMD_SIZE = SIMDNativePixel::size();

/** \brief Gives the number of elements that cannot be processed by a native-width SIMD.
 * \tparam WIDTH the width in pixels.
 */
template<size_t WIDTH>
struct SIMDReminder : std::integral_constant<size_t, WIDTH % SIMD_SIZE>
{};

} // namespace Video

#endif // CDI_VIDEO_SIMD_HPP
