#include <Video/RendererSIMD.hpp>
#include <Video/RendererSoftware.hpp>

#include <array>
#include <chrono>
#include <print>
#include <string_view>

static constexpr size_t WIDTH = 768;
static constexpr size_t FRAMES = 10000;
static constexpr Video::Renderer::DisplayFormat DISPLAY = Video::Renderer::DisplayFormat::PAL;

static constexpr std::array<uint8_t, WIDTH> LINEA{};
static constexpr std::array<uint8_t, WIDTH> LINEB{};

template<typename RENDERER, Video::Renderer::Resolution RESOLUTION>
static void benchmarkRenderer(std::string_view name)
{
    // Configure the renderer.
    RENDERER renderer;
    renderer.SetDisplayResolution(DISPLAY, RESOLUTION);
    renderer.m_codingMethod[Video::Renderer::A] = Video::ImageCodingMethod::CLUT7;
    renderer.m_codingMethod[Video::Renderer::B] = Video::ImageCodingMethod::CLUT8;
    renderer.m_mix = true;

    // Heigt is set by the previous SetDisplayResolution
    const size_t height = renderer.m_plane[Video::Renderer::A].m_height;

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
    benchmarkRenderer<Video::RendererSoftware, Video::Renderer::Resolution::Normal>("Normal Soft");
    benchmarkRenderer<Video::RendererSIMD, Video::Renderer::Resolution::Normal>("Normal SIMD");

    benchmarkRenderer<Video::RendererSoftware, Video::Renderer::Resolution::Double>("Double Soft");
    benchmarkRenderer<Video::RendererSIMD, Video::Renderer::Resolution::Double>("Double SIMD");

    benchmarkRenderer<Video::RendererSoftware, Video::Renderer::Resolution::High>("High   Soft");
    benchmarkRenderer<Video::RendererSIMD, Video::Renderer::Resolution::High>("High   SIMD");
}
