#include <Video/RendererSIMD.hpp>
#include <Video/RendererSoftware.hpp>

#include <array>
#include <chrono>
#include <print>
#include <string_view>

static constexpr size_t WIDTH = 768;
static constexpr size_t HEIGHT = 280;
static constexpr size_t FRAMES = 10000;
static constexpr size_t FRAMES_CURSOR = 10'000'000;

static constexpr std::array<uint8_t, WIDTH> LINEA{};
static constexpr std::array<uint8_t, WIDTH> LINEB{};

static constexpr Video::ImagePlane A = Video::A;
static constexpr Video::ImagePlane B = Video::B;

template<typename RENDERER, Video::Renderer::BitsPerPixel BPS, Video::ImageCodingMethod CODINGA, Video::ImageCodingMethod CODINGB>
static void benchmarkRenderer(std::string_view name)
{
    RENDERER renderer;
    renderer.m_codingMethod[A] = CODINGA;
    renderer.m_codingMethod[B] = CODINGB;
    renderer.m_bps[A] = BPS;
    renderer.m_bps[B] = BPS;
    renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::PAL, false, false);
    renderer.m_mix = true;

    uint16_t lineNumber = 0;

    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < HEIGHT; ++y)
        {
            renderer.DrawLine(LINEA.data(), LINEB.data(), lineNumber++);
        }
        lineNumber = 0;
        renderer.RenderFrame();
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

template<typename RENDERER>
static void benchmarkRendererCursor(std::string_view name)
{
    RENDERER renderer;
    renderer.SetCursorEnabled(true);
    renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::PAL, false, false);

    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES_CURSOR; ++f)
    {
        renderer.RenderFrame();
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::println("{} {}  {}  {}/f  {}",
        name,
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        delta / FRAMES_CURSOR,
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

#if LIBCEDIMU_ENABLE_RENDERERSIMD
#define IF_SIMD(code) code
#else
#define IF_SIMD(code)
#endif

int main()
{
    constexpr Video::Renderer::BitsPerPixel NORMAL_8 = Video::Renderer::BitsPerPixel::Normal8;
    constexpr Video::Renderer::BitsPerPixel DOUBLE_4 = Video::Renderer::BitsPerPixel::Double4;
    // constexpr Video::Renderer::BitsPerPixel HIGH_8 = Video::Renderer::BitsPerPixel::High8;

    benchmarkRendererCursor<Video::RendererSoftware>("Cursor Soft");
    IF_SIMD(benchmarkRendererCursor<Video::RendererSIMD>("Cursor SIMD"));

    benchmarkRenderer<Video::RendererSoftware, NORMAL_8, ICM(OFF), ICM(RGB555)>("Normal Soft RGB555");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, NORMAL_8, ICM(OFF), ICM(RGB555)>("Normal SIMD RGB555")));

    benchmarkRenderer<Video::RendererSoftware, NORMAL_8, ICM(DYUV), ICM(DYUV)>("Normal Soft DYUV");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, NORMAL_8, ICM(DYUV), ICM(DYUV)>("Normal SIMD DYUV")));

    benchmarkRenderer<Video::RendererSoftware, DOUBLE_4, ICM(DYUV), ICM(DYUV)>("Double Soft DYUV");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, DOUBLE_4, ICM(DYUV), ICM(DYUV)>("Double SIMD DYUV")));

    benchmarkRenderer<Video::RendererSoftware, DOUBLE_4, ICM(CLUT4), ICM(CLUT4)>("Double Soft CLUT4");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, DOUBLE_4, ICM(CLUT4), ICM(CLUT4)>("Double SIMD CLUT4")));

    benchmarkRenderer<Video::RendererSoftware, NORMAL_8, ICM(CLUT8), ICM(CLUT7)>("Normal Soft CLUT8/CLUT7");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, NORMAL_8, ICM(CLUT8), ICM(CLUT7)>("Normal SIMD CLUT8/CLUT7")));

    benchmarkRenderer<Video::RendererSoftware, DOUBLE_4, ICM(CLUT8), ICM(CLUT7)>("Double Soft CLUT8/CLUT7");
    IF_SIMD((benchmarkRenderer<Video::RendererSIMD, DOUBLE_4, ICM(CLUT8), ICM(CLUT7)>("Double SIMD CLUT8/CLUT7")));
}
