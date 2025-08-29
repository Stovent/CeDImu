#include <Video/VideoDecoders.hpp>
#include <Video/VideoDecodersSIMD.hpp>

#include <array>
#include <chrono>
#include <print>
#include <string_view>

static constexpr size_t WIDTH = 768;
static constexpr size_t HEIGHT = 560;
static constexpr size_t FRAMES = 20000;
static constexpr size_t FRAMES_DYUV = 5000;

static constexpr std::array<uint8_t, WIDTH> SRCA{};
static constexpr std::array<uint8_t, WIDTH> SRCB{};
static std::array<Video::Pixel, WIDTH> DST{};
static std::array<uint32_t, 256> CLUT{};

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, const uint8_t*, uint16_t) noexcept>
static void benchmarkRGB555Line(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), SRCB.data(), WIDTH);
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

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, uint16_t, uint32_t) noexcept>
static void benchmarkDYUVLine(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES_DYUV; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), WIDTH, 0x0080'1010);
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

template<uint16_t (*DECODE)(Video::Pixel*, const uint8_t*, uint16_t, const uint32_t*, Video::ImageCodingMethod) noexcept>
static void benchmarkCLUTLine(std::string_view name)
{
    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            DECODE(DST.data(), SRCA.data(), WIDTH, CLUT.data(), ICM(CLUT8));
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
    benchmarkRGB555Line<Video::decodeRGB555Line>("RGB555 Soft");
    benchmarkRGB555Line<Video::decodeRGB555LineSIMD>("RGB555 SIMD");

    benchmarkDYUVLine<Video::decodeDYUVLine>("DYUV Soft");
    benchmarkDYUVLine<Video::decodeDYUVLineLUT>("DYUV  LUT");

    benchmarkCLUTLine<Video::decodeCLUTLine>("CLUT Soft");
}
