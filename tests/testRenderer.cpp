#include <catch2/catch_test_macros.hpp>

#include <Video/RendererSoftware.hpp>
#if LIBCEDIMU_ENABLE_RENDERERSIMD
#   include <Video/RendererSIMD.hpp>
#   define IF_SIMD(code) code;
#else
#   define IF_SIMD(code)
#endif

#include <array>

using CursorLine = std::array<Video::Pixel, 16>;
static constexpr Video::Pixel BLACK = Video::Renderer::BLACK_PIXEL;
static constexpr Video::Pixel RED{0xFF'FF'00'00};
static constexpr Video::Pixel CYAN{0xFF'00'FF'FF};
static constexpr Video::Pixel GREEN{0xFF'00'FF'00};
static constexpr Video::Pixel MAGENTA{0xFF'FF'00'FF};

TEST_CASE("Cursor blink", "[Video]")
{
    Video::RendererSoftware rendererSoft;
    IF_SIMD(Video::RendererSIMD rendererSIMD)

    SECTION("Control")
    {
        constexpr CursorLine EXPECTED_ON{
            CYAN, BLACK, CYAN, BLACK, CYAN, BLACK, CYAN, BLACK,
            BLACK, CYAN, BLACK, CYAN, BLACK, CYAN, BLACK, CYAN,
        };
        constexpr CursorLine EXPECTED_OFF{
            BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
            BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
        };

        // No need to test SIMD, as the control code is in Renderer.
        rendererSoft.SetCursorEnabled(true);
        rendererSoft.SetCursorBlink(false, 1, 1);
        rendererSoft.SetCursorPattern(0, 0xAA55);
        rendererSoft.SetCursorColor(0b1011); // Cyan
        rendererSoft.SetDisplayFormat(Video::Renderer::DisplayFormat::PAL, false, false); // 50 FPS.

        rendererSoft.RenderFrame();
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_ON.data()));

        // Advance time to just before the off period.
        rendererSoft.IncrementCursorTime(Video::Renderer::DELTA_50FPS - 1.);
        rendererSoft.RenderFrame();
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_ON.data()));

        // Advance time to exactly the delta time.
        rendererSoft.IncrementCursorTime(1.);
        rendererSoft.RenderFrame();
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_OFF.data()));

        // Make sure the cursor is on when blink is deactivated on an off/complement period.
        rendererSoft.SetCursorBlink(false, 1, 0); // Disable blink.
        rendererSoft.RenderFrame();
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_ON.data()));
    }

    SECTION("ON/OFF")
    {
        rendererSoft.SetCursorEnabled(true);
        rendererSoft.SetCursorBlink(false, 1, 1);
        rendererSoft.SetCursorPattern(0, 0xAA55);
        rendererSoft.SetCursorColor(0b1011); // Cyan
        rendererSoft.RenderFrame();

#if LIBCEDIMU_ENABLE_RENDERERSIMD
        rendererSIMD.SetCursorEnabled(true);
        rendererSIMD.SetCursorBlink(false, 1, 1);
        rendererSIMD.SetCursorPattern(0, 0xAA55);
        rendererSIMD.SetCursorColor(0b1011); // Cyan
        rendererSIMD.RenderFrame();
#endif

        constexpr CursorLine EXPECTED_ON{
            CYAN, BLACK, CYAN, BLACK, CYAN, BLACK, CYAN, BLACK,
            BLACK, CYAN, BLACK, CYAN, BLACK, CYAN, BLACK, CYAN,
        };
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_ON.data()));
        IF_SIMD(REQUIRE(std::equal(rendererSIMD.m_cursorPlane.begin(), rendererSIMD.m_cursorPlane.begin() + 16, EXPECTED_ON.data())))

        rendererSoft.IncrementCursorTime(Video::Renderer::DELTA_50FPS);
        rendererSoft.RenderFrame();
        IF_SIMD(rendererSIMD.IncrementCursorTime(Video::Renderer::DELTA_50FPS))
        IF_SIMD(rendererSIMD.RenderFrame())

        constexpr CursorLine EXPECTED_OFF{
            BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
            BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK, BLACK,
        };
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_OFF.data()));
        IF_SIMD(REQUIRE(std::equal(rendererSIMD.m_cursorPlane.begin(), rendererSIMD.m_cursorPlane.begin() + 16, EXPECTED_OFF.data())))
    }

    SECTION("ON/Complement")
    {
        rendererSoft.SetCursorEnabled(true);
        rendererSoft.SetCursorBlink(true, 1, 1);
        rendererSoft.SetCursorPattern(0, 0xAA55);
        rendererSoft.SetCursorColor(0b1010); // Green
        rendererSoft.RenderFrame();

#if LIBCEDIMU_ENABLE_RENDERERSIMD
        rendererSIMD.SetCursorEnabled(true);
        rendererSIMD.SetCursorBlink(true, 1, 1);
        rendererSIMD.SetCursorPattern(0, 0xAA55);
        rendererSIMD.SetCursorColor(0b1010); // Cyan
        rendererSIMD.RenderFrame();
#endif

        constexpr CursorLine EXPECTED_ON{
            GREEN, BLACK, GREEN, BLACK, GREEN, BLACK, GREEN, BLACK,
            BLACK, GREEN, BLACK, GREEN, BLACK, GREEN, BLACK, GREEN,
        };
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_ON.data()));
        IF_SIMD(REQUIRE(std::equal(rendererSIMD.m_cursorPlane.begin(), rendererSIMD.m_cursorPlane.begin() + 16, EXPECTED_ON.data())))

        rendererSoft.IncrementCursorTime(Video::Renderer::DELTA_50FPS);
        rendererSoft.RenderFrame();
        IF_SIMD(rendererSIMD.IncrementCursorTime(Video::Renderer::DELTA_50FPS))
        IF_SIMD(rendererSIMD.RenderFrame())

        constexpr CursorLine EXPECTED_COMPLEMENT{
            MAGENTA, BLACK, MAGENTA, BLACK, MAGENTA, BLACK, MAGENTA, BLACK,
            BLACK, MAGENTA, BLACK, MAGENTA, BLACK, MAGENTA, BLACK, MAGENTA,
        };
        REQUIRE(std::equal(rendererSoft.m_cursorPlane.begin(), rendererSoft.m_cursorPlane.begin() + 16, EXPECTED_COMPLEMENT.data()));
        IF_SIMD(REQUIRE(std::equal(rendererSIMD.m_cursorPlane.begin(), rendererSIMD.m_cursorPlane.begin() + 16, EXPECTED_COMPLEMENT.data())))
    }
}
