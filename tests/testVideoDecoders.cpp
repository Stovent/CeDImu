#include <catch2/catch_test_macros.hpp>

#include <Video/VideoDecoders.hpp>
#include <Video/VideoDecodersSIMD.hpp>

#include <algorithm>
#include <array>

static constexpr std::array<uint32_t, 256> CLUT = [] {
    std::array<uint32_t, 256> clut{};
    uint32_t i = 0;
    for(uint32_t& color : clut)
    {
        color = (i << 16) | (i << 8) | i;
        ++i;
    }
    return clut;
}();
static_assert(CLUT[1] == 0x0001'0101);
static_assert(CLUT[0xFF] == 0x00FF'FFFF);

static constexpr size_t WIDTH = 768;
using PixelArray = std::array<Video::Pixel, WIDTH>;


static constexpr uint32_t makeColor(uint32_t color) noexcept
{
    color &= 0xFF;
    return (color << 16) | (color << 8) | color;
}

TEST_CASE("RunLength 7", "[Video]")
{
    SECTION("1 pixel 384")
    {
        constexpr std::array<uint8_t, 2> SRC_RL7_1_PIXEL_384{0x8A, 0};
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            std::fill(array.begin(), array.end(), 0x000A'0A0A);
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<384, false>(DST.data(), SRC_RL7_1_PIXEL_384.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("1 pixel 768")
    {
        constexpr std::array<uint8_t, 2> SRC_RL7_1_PIXEL_768{0x91, 0};
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            std::fill(array.begin(), array.end(), 0x0011'1111);
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, false>(DST.data(), SRC_RL7_1_PIXEL_768.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("384 pixels - 28 pixels continuously increasing width")
    {
        // Increasing count from 1 to 27, and last pixel to 0
        constexpr std::array<uint8_t, 55> SRC_RL7_28_PIXEL_384{
            0x01,
            0x82, 2,  0x83, 3,  0x84, 4,  0x85, 5,  0x86, 6,  0x87, 7,  0x88, 8,  0x89, 9,
            0x8A, 10, 0x8B, 11, 0x8C, 12, 0x8D, 13, 0x8E, 14, 0x8F, 15, 0x90, 16, 0x91, 17,
            0x92, 18, 0x93, 19, 0x94, 20, 0x95, 21, 0x96, 22, 0x97, 23, 0x98, 24, 0x99, 25,
            0x9A, 26, 0x9B, 27, 0x9C, 0,
        };
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{0x0001'0101, 0x0001'0101}; // First pixel
            size_t index = 2;
            for(size_t count = 2; count <= 27; ++count)
            {
                for(size_t i = 0; i < count; ++i)
                {
                    const uint32_t color = makeColor(count);
                    array.at(index++) = color;
                    array.at(index++) = color;
                }
            }
            while(index < 768)
            {
                const uint32_t color = makeColor(0x1C);
                array.at(index++) = color;
                array.at(index++) = color;
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<384, false>(DST.data(), SRC_RL7_28_PIXEL_384.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("768 pixels - 39 pixels continuously increasing width")
    {
        // Increasing count from 1 to 38, and last pixel to 0
        constexpr std::array<uint8_t, 77> SRC_RL7_39_PIXEL_768{
            0x01,
            0x82, 2,  0x83, 3,  0x84, 4,  0x85, 5,  0x86, 6,  0x87, 7,  0x88, 8,  0x89, 9,
            0x8A, 10, 0x8B, 11, 0x8C, 12, 0x8D, 13, 0x8E, 14, 0x8F, 15, 0x90, 16, 0x91, 17,
            0x92, 18, 0x93, 19, 0x94, 20, 0x95, 21, 0x96, 22, 0x97, 23, 0x98, 24, 0x99, 25,
            0x9A, 26, 0x9B, 27, 0x9C, 28, 0x9D, 29, 0x9E, 30, 0x9F, 31, 0xA0, 32, 0xA1, 33,
            0xA2, 34, 0xA3, 35, 0xA4, 36, 0xA5, 37, 0xA6, 38, 0xA7, 0,
        };
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{0x0001'0101}; // First pixel
            size_t index = 1;
            for(size_t count = 2; count <= 38; ++count)
            {
                for(size_t i = 0; i < count; ++i)
                {
                    array.at(index++) = makeColor(count);
                }
            }
            while(index < 768)
            {
                array.at(index++) = makeColor(0x27);
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, false>(DST.data(), SRC_RL7_39_PIXEL_768.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("384 individual pixels")
    {
        constexpr std::array<uint8_t, 384> SRC_RL7_384_PIXEL_384 = [] {
            std::array<uint8_t, 384> array{};
            for(size_t i = 0; i < array.size(); ++i)
            {
                array.at(i) = i & 0x7F;
            }
            return array;
        }();
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            for(size_t i = 0; i < array.size();)
            {
                const uint32_t color = makeColor((i / 2) & 0x7F);
                array.at(i++) = color;
                array.at(i++) = color;
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<384, false>(DST.data(), SRC_RL7_384_PIXEL_384.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("768 individual pixels")
    {
        constexpr std::array<uint8_t, 768> SRC_RL7_768_PIXEL_768 = [] {
            std::array<uint8_t, 768> array{};
            for(size_t i = 0; i < array.size(); ++i)
            {
                array.at(i) = i & 0x7F;
            }
            return array;
        }();
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            for(size_t i = 0; i < array.size(); ++i)
            {
                array.at(i) = makeColor(i & 0x7F);
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, false>(DST.data(), SRC_RL7_768_PIXEL_768.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }
}

TEST_CASE("RunLength 3", "[Video]")
{
    SECTION("1 pixel")
    {
        constexpr std::array<uint8_t, 2> SRC_RL3_1_PIXEL{0x92, 0};
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            for(size_t i = 0; i < array.size();)
            {
                array.at(i++) = 0x0001'0101;
                array.at(i++) = 0x0002'0202;
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, true>(DST.data(), SRC_RL3_1_PIXEL.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("28 pixels continuously increasing width")
    {
        // Increasing count from 1 to 27, and last pixel to 0
        constexpr std::array<uint8_t, 55> SRC_RL3_28_PIXEL{
            0x01,
            0x82, 2,  0x83, 3,  0x84, 4,  0x85, 5,  0x86, 6,  0x87, 7,  0x80, 8,  0x81, 9,
            0x82, 10, 0x83, 11, 0x84, 12, 0x85, 13, 0x86, 14, 0x87, 15, 0x90, 16, 0x91, 17,
            0x92, 18, 0x93, 19, 0x94, 20, 0x95, 21, 0x96, 22, 0x97, 23, 0x90, 24, 0x91, 25,
            0x92, 26, 0x93, 27, 0x94, 0,
        };
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{0x0000'0000, 0x0001'0101}; // First pixel
            size_t index = 2;
            for(size_t count = 2; count <= 27; ++count)
            {
                for(size_t i = 0; i < count; ++i)
                {
                    if(count <= 15)
                        array.at(index++) = 0x0000'0000;
                    else
                        array.at(index++) = 0x0001'0101;
                    array.at(index++) = makeColor(count & 0x07);
                }
            }
            while(index < 768)
            {
                array.at(index++) = 0x0001'0101;
                array.at(index++) = makeColor(4);
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, true>(DST.data(), SRC_RL3_28_PIXEL.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }

    SECTION("768 individual pixels")
    {
        constexpr std::array<uint8_t, 384> SRC_RL3_768_PIXEL = [] {
            std::array<uint8_t, 384> array{};
            for(size_t i = 0; i < array.size(); ++i)
            {
                array.at(i) = 0x08 | (i << 4 & 0x70) | ((i + 1) & 0x07);
            }
            return array;
        }();
        constexpr PixelArray EXPECTED = [] {
            PixelArray array{};
            for(size_t i = 0; i < array.size();)
            {
                uint32_t color1 = (i / 2) & 0x07;
                uint32_t color2 = ((i / 2) + 1) & 0x07;
                array.at(i++) = makeColor(color1);
                array.at(i++) = makeColor(color2);
            }
            return array;
        }();
        PixelArray DST{};

        Video::decodeRunLengthLine<WIDTH, true>(DST.data(), SRC_RL3_768_PIXEL.data(), CLUT.data());
        REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
    }
}

TEST_CASE("RGB555", "[Video]")
{
    constexpr std::array<uint8_t, 384> LINEA = [] {
        std::array<uint8_t, 384> array{};
        for(size_t i = 0; i < array.size();)
        {
            const uint32_t color = 0x80 | (i << 2 & 0x7C) | (i >> 3 & 0x03);
            array.at(i++) = color;
        }
        return array;
    }();
    constexpr std::array<uint8_t, 384> LINEB = [] {
        std::array<uint8_t, 384> array{};
        for(size_t i = 0; i < array.size();)
        {
            const uint8_t color = (i << 5) | (i & 0x1F);
            array.at(i++) = color;
        }
        return array;
    }();
    constexpr PixelArray EXPECTED = [] {
        PixelArray array{};
        for(size_t i = 0; i < array.size(); i += 2)
        {
            const uint32_t color = makeColor(((i / 2) << 3) & 0xF8);
            array.at(i) = color;
            array.at(i + 1) = color;
        }
        return array;
    }();
    PixelArray DST{};

    Video::decodeRGB555Line<384>(DST.data(), LINEA.data(), LINEB.data());
    REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));

#if LIBCEDIMU_ENABLE_RENDERERSIMD
    Video::decodeRGB555LineSIMD<384>(DST.data(), LINEA.data(), LINEB.data());
    REQUIRE(std::equal(DST.cbegin(), DST.cend(), EXPECTED.data()));
#endif
}
