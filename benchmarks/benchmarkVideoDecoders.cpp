#include <Video/VideoDecoders.hpp>
#include <Video/VideoDecodersSIMD.hpp>

#include <array>
#include <chrono>
#include <print>
#include <string_view>

static constexpr size_t HALF_WIDTH = 384;
static constexpr size_t WIDTH = 768;
static constexpr size_t HEIGHT = 560;
static constexpr size_t FRAMES = 20000;
static constexpr size_t FRAMES_DYUV = 5000;

static std::array<uint32_t, 256> CLUT{};
static std::array<Video::Pixel, WIDTH> DST{};
static constexpr std::array<uint8_t, WIDTH> SRCA{};
static constexpr std::array<uint8_t, WIDTH> SRCB{};
static constexpr std::array<uint8_t, 2> SRC_RL_1_PIXEL{0x80, 0};
static constexpr std::array<uint8_t, 384> SRC_RL7_384_PIXEL_384 = [] {
    std::array<uint8_t, 384> array{};
    for(size_t i = 0; i < array.size(); ++i)
    {
        array.at(i) = i & 0x7F;
    }
    return array;
}();
static constexpr std::array<uint8_t, 768> SRC_RL7_768_PIXEL_768 = [] {
    std::array<uint8_t, 768> array{};
    for(size_t i = 0; i < array.size(); ++i)
    {
        array.at(i) = i & 0x7F;
    }
    return array;
}();
static constexpr std::array<uint8_t, 384> SRC_RL3_768_PIXEL = [] {
    std::array<uint8_t, 384> array{};
    for(size_t i = 0; i < array.size(); ++i)
    {
        array.at(i) = 0x08 | (i << 4 & 0x70) | ((i + 1) & 0x07);
    }
    return array;
}();

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, const uint8_t*) noexcept>
static void benchmarkRGB555Line(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), SRCB.data());
        }
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::println("{} {}  {}  {}/f  {}",
        name,
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        std::chrono::duration_cast<std::chrono::microseconds>(delta / FRAMES),
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, uint32_t) noexcept>
static void benchmarkDYUVLine(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES_DYUV; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), 0x0080'1010);
        }
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::println("{} {}  {}  {}/f  {}",
        name,
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        std::chrono::duration_cast<std::chrono::microseconds>(delta / FRAMES_DYUV),
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, const uint32_t*, Video::ImageCodingMethod) noexcept>
static void benchmarkCLUTLine(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), CLUT.data(), ICM(CLUT8));
        }
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::println("{} {}  {}  {}/f  {}",
        name,
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        std::chrono::duration_cast<std::chrono::microseconds>(delta / FRAMES),
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, const uint32_t*) noexcept>
static void benchmarkRunLength(std::string_view name, const uint8_t* data)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), data, CLUT.data());
        }
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::println("{} {}  {}  {}/f  {}",
        name,
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        std::chrono::duration_cast<std::chrono::microseconds>(delta / FRAMES),
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

int main()
{
    benchmarkRGB555Line<Video::decodeRGB555Line<HALF_WIDTH>>("RGB555 Soft");
    benchmarkRGB555Line<Video::decodeRGB555LineSIMD<HALF_WIDTH>>("RGB555 SIMD");

    benchmarkDYUVLine<Video::decodeDYUVLine<HALF_WIDTH>>("DYUV Soft");
    benchmarkDYUVLine<Video::decodeDYUVLineLUT<HALF_WIDTH>>("DYUV  LUT");
    benchmarkDYUVLine<Video::decodeDYUVLineSIMD<HALF_WIDTH>>("DYUV SIMD");

    benchmarkCLUTLine<Video::decodeCLUTLine<WIDTH>>("CLUT Soft");

    benchmarkRunLength<Video::decodeRunLengthLine<360, false>>("RL7 normal   1 pixel ", SRC_RL_1_PIXEL.data());
    benchmarkRunLength<Video::decodeRunLengthLine<360, false>>("RL7 normal 384 pixels", SRC_RL7_384_PIXEL_384.data());
    benchmarkRunLength<Video::decodeRunLengthLine<720, false>>("RL7 high     1 pixel ", SRC_RL_1_PIXEL.data());
    benchmarkRunLength<Video::decodeRunLengthLine<720, false>>("RL7 high   768 pixels", SRC_RL7_768_PIXEL_768.data());
    benchmarkRunLength<Video::decodeRunLengthLine<720, true>>("RL3 double   1 pixel ", SRC_RL_1_PIXEL.data());
    benchmarkRunLength<Video::decodeRunLengthLine<720, true>>("RL3 double 384 pixels", SRC_RL3_768_PIXEL.data());
}
