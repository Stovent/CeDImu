#include "VideoDecodersSIMD.hpp"
#include "SIMD.hpp"
#include "VideoDecoders.hpp"
#include "common/panic.hpp"
#include "common/utils.hpp"

#include <algorithm>
#include <bit>
#include <execution>
#include <iterator>

namespace Video
{

/** \brief Decode a bitmap file line to double resolution ARGB.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the decoded line will be written to in ARGB.
 * \param dataA See below.
 * \param dataB See below.
 * \param CLUTTable The CLUT table to use.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \param icm The coding method of the file.
 * \return The number of raw bytes read from dataB (and dataA in RGB555).
 *
 * If \p icm is ImageCodingMethod::CLUT77 or the video plane is plane B, then send `&CLUT[128]` as the \p CLUTTable.
 *
 * If the coding method is RGB555, dataA must contain the channel A data and dataB must contain the channel B data.
 * If it is not RGB555, dataB is the source data and dataA remain unused.
 *
 * \warning The pixels are always decoded to double resolution (720 or 768 pixels), no matter the source width.
 * When the source width is normal resolution, the pixels are simply duplicated.
 */
template<uint16_t WIDTH>
uint16_t decodeBitmapLineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept
{
    if(icm == ImageCodingMethod::DYUV)
        // return decodeDYUVLineSIMD<WIDTH>(dst, dataB, initialDYUV);
        return decodeDYUVLineLUT<WIDTH>(dst, dataB, initialDYUV);

    if(icm == ImageCodingMethod::RGB555)
    {
        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
            return decodeRGB555LineSIMD<WIDTH>(dst, dataA, dataB);
        }
        else
        {
            panic("RGB555 is only usable in normal resolution");
        }
    }

    return decodeCLUTLine<WIDTH>(dst, dataB, CLUTTable, icm);
}
template uint16_t decodeBitmapLineSIMD<360>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLineSIMD<384>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLineSIMD<720>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;
template uint16_t decodeBitmapLineSIMD<768>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB, const uint32_t* CLUTTable, uint32_t initialDYUV, ImageCodingMethod icm) noexcept;

/** \brief Decode a RGB555 line to ARGB using SIMD.
 * \tparam WIDTH The number or source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param dataA The plane A data (high order byte of the pixel).
 * \param dataB The plane B data (low order byte of the pixel).
 * \return The number of raw bytes read from each data source.
 * \attention \p dst, \p dataA and \p dataB are written/read in chunks of std::simd::size(). Make sure the buffers can be read and written beyond the actual line length.
 * The transparency bit is set in the alpha byte (0x80 when the bit is 1, 0 otherwise).
 */
template<uint16_t WIDTH>
uint16_t decodeRGB555LineSIMD(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept
{
#if LIBCEDIMU_RENDERERSIMD_STD
    using SIMDFixedU64 = std::simd::rebind_t<uint64_t, SIMDNativePixel>;

    const SIMDNativePixel alphaMask{0x8000u};
    size_t width = WIDTH;
    // TODO: can we optimize with simd bit operations?
    while(width > 0)
    {
        const uint16_t count = std::min(SIMDNativePixelSize, width);

        const SIMDNativePixel da = std::simd::partial_load<SIMDNativePixel>(dataA, count);
        const SIMDNativePixel db = std::simd::partial_load<SIMDNativePixel>(dataB, count);
        const SIMDNativePixel data = da << 8 | db;

        SIMDNativePixel result32 = (data & alphaMask) << 16;
        result32 |= data << 9 & 0x00F8'0000u;
        result32 |= data << 6 & 0x0000'F800u;
        result32 |= data << 3 & 0x0000'00F8u;

        // Duplicate the pixels.
        SIMDFixedU64 result64 = result32;
        result64 |= result64 << 32;

        // This reinterpret_cast should be safe, as the overlaligned tag should make the write safe.
        // Also casting to a uint64_t pointer should not affect endianness.
        std::simd::partial_store(result64, reinterpret_cast<uint64_t*>(dst->AsU32Pointer()), count, std::simd::flag_overaligned<alignof(Pixel::ARGB32)>);

        width -= count;
        dataA += SIMDNativePixelSize;
        dataB += SIMDNativePixelSize;
        dst += SIMDNativePixelSize * 2;
    };
#else
    // TODO: ensure we do not index out of bound.
    using SIMDFixedU32 = stdx::rebind_simd_t<uint32_t, SIMDNativeU8>;
    using SIMDFixedU64 = stdx::rebind_simd_t<uint64_t, SIMDFixedU32>;

    const uint8_t* endA = dataA + WIDTH;

    const SIMDFixedU32 alphaMask{0x8000};
    for(; dataA < endA; dataA += SIMDNativeU8::size(), dataB += SIMDNativeU8::size(), dst += SIMDNativeU8::size() * 2)
    {
        const SIMDNativeU8 da{dataA, stdx::element_aligned};
        const SIMDNativeU8 db{dataB, stdx::element_aligned};

        const SIMDFixedU32 a = stdx::static_simd_cast<uint32_t>(da);
        const SIMDFixedU32 b = stdx::static_simd_cast<uint32_t>(db);
        const SIMDFixedU32 data = a << 8 | b;

        SIMDFixedU32 result32 = (data & alphaMask) << 16;
        result32 |= data << 9 & 0x00F8'0000;
        result32 |= data << 6 & 0x0000'F800;
        result32 |= data << 3 & 0x0000'00F8;

        // Duplicate the pixels.
        SIMDFixedU64 result64 = stdx::static_simd_cast<uint64_t>(result32);
        result64 |= result64 << 32;

        // This reinterpret_cast should be safe, as the overlaligned tag should make the write safe.
        // Also casting to a uint64_t pointer should not affect endianness.
        result64.copy_to(reinterpret_cast<uint64_t*>(dst->AsU32Pointer()), stdx::overaligned<alignof(Pixel::ARGB32)>);
    };
#endif

    return WIDTH;
}
template uint16_t decodeRGB555LineSIMD<360>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;
template uint16_t decodeRGB555LineSIMD<384>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept;
template<> uint16_t decodeRGB555LineSIMD<720>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept = delete;
template<> uint16_t decodeRGB555LineSIMD<768>(Pixel* dst, const uint8_t* dataA, const uint8_t* dataB) noexcept = delete;

#if LIBCEDIMU_RENDERERSIMD_STD
using SIMDNativeI32 = std::simd::vec<int32_t>;
using SIMDFixedI64 = std::simd::rebind_t<int64_t, SIMDNativeI32>;
#else
using SIMDNativeI32 = stdx::native_simd<int32_t>;
using SIMDFixedU64 = stdx::rebind_simd_t<uint64_t, SIMDNativeI32>;
#endif
static inline constexpr size_t SIZE = SIMDNativeI32::size();
static_assert(SIZE == SIMD_SIZE);
static inline constexpr SIMDNativeI32 U8_MIN{0};
static inline constexpr SIMDNativeI32 U8_MAX{255};

/** \brief Decode a DYUV line to ARGB using SIMD.
 * \tparam WIDTH The number of source pixels to decode.
 * \param dst Where the ARGB data will be written to.
 * \param dyuv The source DYUV data.
 * \param initialDYUV The initial value to be used by the DYUV decoder.
 * \return The number of raw bytes read from \p dyuv.
 *
 * Because each pixel depends on the previous one, the dequantization and UV interpolation must be done sequentially.
 * However the RGB matrixing can be parallel.
 */
template<uint16_t WIDTH>
uint16_t decodeDYUVLineSIMD(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept
{
#if LIBCEDIMU_RENDERERSIMD_STD
    using STORE_YUV = uint8_t;
#else
    using STORE_YUV = int32_t;
#endif

    std::array<STORE_YUV, WIDTH> y;
    std::array<STORE_YUV, WIDTH> u;
    std::array<STORE_YUV, WIDTH> v;

    uint8_t py = bits<16, 23>(initialDYUV);
    uint8_t pu = bits<8, 15>(initialDYUV);
    uint8_t pv = initialDYUV;

    for(uint16_t index = 0; index < WIDTH; index += 2)
    {
        const uint8_t high = dyuv[index];
        const uint8_t low = dyuv[index + 1];

        // Green book V.4.4.2
        uint8_t u2 = bits<4, 7>(high);
        uint8_t y1 = bits<0, 3>(high);
        uint8_t v2 = bits<4, 7>(low);
        uint8_t y2 = bits<0, 3>(low);

        y1 = py + dequantizer[y1];
        u2 = pu + dequantizer[u2];
        v2 = pv + dequantizer[v2];
        y2 = y1 + dequantizer[y2];
        const uint8_t u1 = (as<uint16_t>(pu) + as<uint16_t>(u2)) >> 1;
        const uint8_t v1 = (as<uint16_t>(pv) + as<uint16_t>(v2)) >> 1;

        // Store previous.
        py = y2;
        pu = u2;
        pv = v2;

        y[index] = y1;
        u[index] = u1;
        v[index] = v1;
        y[index + 1] = y2;
        u[index + 1] = u2;
        v[index + 1] = v2;
    }

    const STORE_YUV* Y = y.data();
    const STORE_YUV* U = u.data();
    const STORE_YUV* V = v.data();

    // TODO: make sure we do not write out of range.
    for(uint16_t i = 0; i < WIDTH;
        i += SIZE, Y += SIZE, U += SIZE, V += SIZE)
    {
#if LIBCEDIMU_RENDERERSIMD_STD
        SIMDNativeI32 simdY = std::simd::unchecked_load<SIMDNativeI32>(Y, SIZE);
        SIMDNativeI32 simdU = std::simd::unchecked_load<SIMDNativeI32>(U, SIZE);
        SIMDNativeI32 simdV = std::simd::unchecked_load<SIMDNativeI32>(V, SIZE);
#else
        SIMDNativeI32 simdY{Y, stdx::element_aligned};
        SIMDNativeI32 simdU{U, stdx::element_aligned};
        SIMDNativeI32 simdV{V, stdx::element_aligned};
#endif

        simdU -= 128;
        simdV -= 128;

        // SIMDNativeI32 simdR = ((simdV * 351) >> 8) + simdY;
        SIMDNativeI32 simdR = ((simdV * 351) / 256) + simdY;
        // SIMDNativeI32 simdG = (((simdU * 86) + (simdV * 179)) >> 8) + simdY;
        SIMDNativeI32 simdG = (((simdU * 86) + (simdV * 179)) / 256) + simdY;
        // SIMDNativeI32 simdB = ((simdU * 444) >> 8) + simdY;
        SIMDNativeI32 simdB = ((simdU * 444) / 256) + simdY;

#if LIBCEDIMU_RENDERERSIMD_STD
        simdR = std::simd::clamp(simdR, U8_MIN, U8_MAX);
        simdG = std::simd::clamp(simdG, U8_MIN, U8_MAX);
        simdB = std::simd::clamp(simdB, U8_MIN, U8_MAX);
#else
        simdR = stdx::clamp(simdR, U8_MIN, U8_MAX);
        simdG = stdx::clamp(simdG, U8_MIN, U8_MAX);
        simdB = stdx::clamp(simdB, U8_MIN, U8_MAX);
#endif

        const SIMDNativeI32 result32 = (simdR << 16) | (simdG << 8) | simdB;

        if constexpr(WIDTH == 360 || WIDTH == 384)
        {
#if LIBCEDIMU_RENDERERSIMD_STD
            SIMDFixedI64 result64 = result32;
            result64 |= result64 << 32;
            std::simd::unchecked_store(result64, reinterpret_cast<int64_t*>(dst->AsU32Pointer()), SIZE, std::simd::flag_overaligned<alignof(uint32_t)>);
#else
            SIMDFixedU64 result64 = stdx::static_simd_cast<uint64_t>(result32);
            result64 |= result64 << 32;
            result64.copy_to(reinterpret_cast<uint64_t*>(dst->AsU32Pointer()), stdx::element_aligned);
#endif
            dst += SIZE * 2;
        }
        else
        {
#if LIBCEDIMU_RENDERERSIMD_STD
            std::simd::unchecked_store(result32, reinterpret_cast<int32_t*>(dst->AsU32Pointer()), SIZE);
#else
            result32.copy_to(dst->AsU32Pointer(), stdx::element_aligned);
#endif
            dst += SIZE;
        }
    }

    return WIDTH;
}
template uint16_t decodeDYUVLineSIMD<360>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineSIMD<384>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineSIMD<720>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;
template uint16_t decodeDYUVLineSIMD<768>(Pixel* dst, const uint8_t* dyuv, uint32_t initialDYUV) noexcept;

} // namespace Video
