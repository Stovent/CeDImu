#include <Video/RendererSIMD.hpp>
#include <Video/RendererSoftware.hpp>

#include <array>
#include <chrono>
#include <print>
#include <string_view>

static constexpr size_t WIDTH = 768;
static constexpr size_t FRAMES = 10000;

static constexpr std::array<uint8_t, WIDTH> LINEA{};
static constexpr std::array<uint8_t, WIDTH> LINEB{};

static constexpr Video::Renderer::ImagePlane A = Video::Renderer::A;
static constexpr Video::Renderer::ImagePlane B = Video::Renderer::B;

template<typename RENDERER, Video::Renderer::BitsPerPixel BPS, Video::ImageCodingMethod CODINGA, Video::ImageCodingMethod CODINGB>
static void benchmarkRenderer(std::string_view name)
{    // Configure the renderer.
    RENDERER renderer;
    renderer.m_codingMethod[A] = CODINGA;
    renderer.m_codingMethod[B] = CODINGB;
    renderer.m_bps[A] = BPS;
    renderer.m_bps[B] = BPS;
    renderer.m_mix = true;

    const size_t height = renderer.m_plane[A].m_height;

    // Benchmark
    const std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
    for(size_t f = 0; f < FRAMES; ++f)
    {
        for(size_t y = 0; y < height; ++y)
        {
            renderer.DrawLine(LINEA.data(), LINEB.data());
        }
        // renderer.m_lineNumber = 0;
        renderer.RenderFrame();
    }
    const std::chrono::high_resolution_clock::time_point finish = std::chrono::high_resolution_clock::now();
    const std::chrono::nanoseconds delta = finish - start;

    std::print("{} ", name);
    std::println("{}  {}  {}/f  {}",
        delta,
        std::chrono::duration_cast<std::chrono::microseconds>(delta),
        std::chrono::duration_cast<std::chrono::microseconds>(delta / FRAMES),
        std::chrono::duration_cast<std::chrono::milliseconds>(delta)
    );
}

int main()
{
    constexpr Video::Renderer::BitsPerPixel normal8 = Video::Renderer::BitsPerPixel::Normal8;
    constexpr Video::Renderer::BitsPerPixel double4 = Video::Renderer::BitsPerPixel::Double4;
    constexpr Video::Renderer::BitsPerPixel high8 = Video::Renderer::BitsPerPixel::High8;

    benchmarkRenderer<Video::RendererSoftware, normal8, ICM(OFF), ICM(RGB555)>("Normal Soft RGB555");
    benchmarkRenderer<Video::RendererSIMD, normal8, ICM(OFF), ICM(RGB555)>("Normal SIMD RGB555");

    benchmarkRenderer<Video::RendererSoftware, normal8, ICM(DYUV), ICM(DYUV)>("Normal Soft DYUV");
    benchmarkRenderer<Video::RendererSIMD, normal8, ICM(DYUV), ICM(DYUV)>("Normal SIMD DYUV");

    benchmarkRenderer<Video::RendererSoftware, double4, ICM(CLUT4), ICM(CLUT4)>("Double Soft CLUT4");
    benchmarkRenderer<Video::RendererSIMD, double4, ICM(CLUT4), ICM(CLUT4)>("Double SIMD CLUT4");

    benchmarkRenderer<Video::RendererSoftware, high8, ICM(CLUT8), ICM(CLUT7)>("High   Soft CLUT8/CLUT7");
    benchmarkRenderer<Video::RendererSIMD, high8, ICM(CLUT8), ICM(CLUT7)>("High   SIMD CLUT8/CLUT7");
}
