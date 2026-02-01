#include <catch2/catch_test_macros.hpp>

#include <Video/RendererSoftware.hpp>
#if LIBCEDIMU_ENABLE_RENDERERSIMD
#   include <Video/RendererSIMD.hpp>
#   define IF_SIMD(code) code;
#else
#   define IF_SIMD(code)
#endif

#include <algorithm>
#include <array>

using CursorLine = std::array<Video::Pixel, 16>;
inline constexpr Video::Pixel BLACK = Video::Renderer::BLACK_PIXEL;
inline constexpr Video::Pixel RED{0xFF'FF'00'00};
inline constexpr Video::Pixel HALF_RED{0xFF'85'09'09};
inline constexpr Video::Pixel HIDDEN_RED{0x00'FF'00'00};
inline constexpr Video::Pixel CYAN{0xFF'00'FF'FF};
inline constexpr Video::Pixel GREEN{0xFF'00'FF'00};
inline constexpr Video::Pixel HALF_GREEN{0xFF'09'85'09};
inline constexpr Video::Pixel HIDDEN_GREEN{0x00'00'FF'00};
inline constexpr Video::Pixel MAGENTA{0xFF'FF'00'FF};
inline constexpr Video::Pixel BLUE{0xFF'00'00'FF}; // Background color.

inline constexpr Video::ImagePlane PLANEA = Video::A;
inline constexpr Video::ImagePlane PLANEB = Video::B;

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

static constexpr uint32_t makeCommand(uint32_t op, bool mf, uint32_t icf, uint32_t pos) noexcept
{
    return (op << 20) | (static_cast<uint32_t>(mf) << 16) | (icf << 10) | pos;
}

static constexpr void configureOneMatte(Video::Renderer& renderer) noexcept
{
    renderer.m_transparencyControl[PLANEA] = 0b0100; // Matte flag 1 = true.
    renderer.m_transparencyControl[PLANEB] = 0b1000; // Never.
    renderer.m_mix = false;
    renderer.m_planeOrder = false; // A in front of B.

    renderer.m_matteNumber = false;
    renderer.m_icf[PLANEA] = 63;
    renderer.m_icf[PLANEB] = 63;
    // starts with matte flags false.
    renderer.m_matteControl[0] = makeCommand(0b0100, true, 63, 0); // ICF A 63.
    renderer.m_matteControl[1] = makeCommand(0b0110, true, 63, 1); // ICF B 63.
    renderer.m_matteControl[2] = makeCommand(0b0100, true, 31, 2); // ICF A 31.
    renderer.m_matteControl[3] = makeCommand(0b1001, true, 0, 3); // Set matte flag 1, plane A hidden.
    renderer.m_matteControl[4] = makeCommand(0b0110, true, 31, 5); // ICF B 31.
    renderer.m_matteControl[5] = makeCommand(0b1000, true, 0, 729); // Reset matte flag 1, plane A visible.
    renderer.m_matteControl[6] = makeCommand(0, true, 0, 730); // Ignore later commands.
    renderer.m_matteControl[7] = makeCommand(0b0100, true, 0, 740); // ICF A 0, must be ignored.
}

static constexpr void configureTwoMattes(Video::Renderer& renderer) noexcept
{
    renderer.m_transparencyControl[PLANEA] = 0b1100; // Matte flag 1 = false.
    renderer.m_transparencyControl[PLANEB] = 0b1011; // Matte flag 0 = false.
    renderer.m_mix = false;
    renderer.m_planeOrder = true; // B in front of A.

    renderer.m_matteNumber = true;
    renderer.m_icf[PLANEA] = 63;
    renderer.m_icf[PLANEB] = 63;
    // starts with matte flags false.
    // mf bit must be ignored.
    renderer.m_matteControl[0] = makeCommand(0b1111, true, 31, 8); // Set matte flag 0, ICF B 31, plane B visible.
    renderer.m_matteControl[2] = makeCommand(0, true, 0, 3); // Ignore later commands.
    renderer.m_matteControl[3] = makeCommand(0b1000, true, 0, 11); // ICF B 0, must be ignored.
    renderer.m_matteControl[3] = makeCommand(0b1000, true, 63, 12); // ICF B 63, must be ignored.

    renderer.m_matteControl[4] = makeCommand(0b1101, false, 63, 2); // Set matte flag 1, ICF A 63, plane A visible.
    renderer.m_matteControl[5] = makeCommand(0b0100, false, 31, 4); // ICF A 31.
    renderer.m_matteControl[6] = makeCommand(0, false, 63, 6); // Ignore.
    renderer.m_matteControl[7] = makeCommand(0b0110, false, 63, 8); // ICF B 63, ignored.
}

static void configureCLUT(Video::Renderer& renderer) noexcept
{
    renderer.m_backdropColor = 0b1001; // Blue.
    renderer.m_codingMethod[PLANEA] = ICM(CLUT7);
    renderer.m_codingMethod[PLANEB] = ICM(CLUT7);
    renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::PAL, false, false);
    renderer.m_clut[0] = RED.AsU32();
    renderer.m_clut[128] = GREEN.AsU32();
}

static constexpr std::array<uint8_t, 384> INPUT_A = [] () {
    std::array<uint8_t, 384> array{};
    std::ranges::fill(array, 0); // CLUT 0
    return array;
}();

static constexpr std::array<uint8_t, 384> INPUT_B = [] () {
    std::array<uint8_t, 384> array{};
    std::ranges::fill(array, 0); // CLUT 0 + 128
    return array;
}();

TEST_CASE("Matte", "[Video]")
{
    Video::RendererSoftware rendererSoft;
    IF_SIMD(Video::RendererSIMD rendererSIMD)

    SECTION("One matte")
    {
        configureOneMatte(rendererSoft);
        IF_SIMD(configureOneMatte(rendererSIMD))

        configureCLUT(rendererSoft);
        IF_SIMD(configureCLUT(rendererSIMD))

        rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        IF_SIMD(rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0))

        constexpr std::array<Video::Pixel, 768> EXPECTED_A = [] () {
            std::array<Video::Pixel, 768> array{RED, RED, RED};
            std::fill(array.data() + 3, array.data() + 729, HIDDEN_RED);
            std::fill(array.begin() + 729, array.end(), RED);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_B = [] () {
            std::array<Video::Pixel, 768> array;
            std::ranges::fill(array, GREEN);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_SCREEN = [] () {
            std::array<Video::Pixel, 768> array{
                RED, RED, HALF_RED, GREEN, GREEN,
            };
            std::fill(array.data() + 5, array.data() + 729, HALF_GREEN);
            std::fill(array.begin() + 729, array.end(), HALF_RED);
            return array;
        }();

        REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSoft.m_plane[PLANEA].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSIMD.m_plane[PLANEA].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSoft.m_plane[PLANEB].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSIMD.m_plane[PLANEB].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSoft.m_screen.GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSIMD.m_screen.GetLinePointer(0))))
    }

    SECTION("Two matte")
    {
        configureTwoMattes(rendererSoft);
        IF_SIMD(configureTwoMattes(rendererSIMD))

        configureCLUT(rendererSoft);
        IF_SIMD(configureCLUT(rendererSIMD))

        rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        IF_SIMD(rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0))

        constexpr std::array<Video::Pixel, 768> EXPECTED_A = [] () {
            std::array<Video::Pixel, 768> array{
                HIDDEN_RED, HIDDEN_RED, RED, RED,
            };
            std::fill(array.begin() + 4, array.end(), RED);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_B = [] () {
            std::array<Video::Pixel, 768> array;
            std::fill(array.begin(), array.begin() + 8, HIDDEN_GREEN);
            std::fill(array.begin() + 8, array.end(), GREEN);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_SCREEN = [] () {
            std::array<Video::Pixel, 768> array{
                BLUE, BLUE, RED, RED, HALF_RED, HALF_RED, HALF_RED, HALF_RED,
            };
            std::fill(array.begin() + 8, array.end(), HALF_GREEN);
            return array;
        }();

        REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSoft.m_plane[PLANEA].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSIMD.m_plane[PLANEA].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSoft.m_plane[PLANEB].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSIMD.m_plane[PLANEB].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSoft.m_screen.GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSIMD.m_screen.GetLinePointer(0))))
    }
}

static constexpr void configureOneMatteLower(Video::Renderer& renderer) noexcept
{
    renderer.m_transparencyControl[PLANEA] = 0b0100; // Matte flag 1 = true.
    renderer.m_transparencyControl[PLANEB] = 0b1000; // Never.
    renderer.m_mix = false;
    renderer.m_planeOrder = false; // A in front of B.

    renderer.m_matteNumber = false;
    renderer.m_icf[PLANEA] = 63;
    renderer.m_icf[PLANEB] = 63;
    // starts with matte flags false.
    renderer.m_matteControl[0] = makeCommand(0b0100, true, 63, 0); // ICF A 63.
    renderer.m_matteControl[1] = makeCommand(0b0110, true, 63, 1); // ICF B 63.
    renderer.m_matteControl[2] = makeCommand(0b0100, true, 31, 2); // ICF A 31.
    renderer.m_matteControl[3] = makeCommand(0b1001, true, 0, 3); // Set matte flag 1, plane A hidden.
    renderer.m_matteControl[4] = makeCommand(0b0110, true, 31, 4); // ICF B 31.
    renderer.m_matteControl[5] = makeCommand(0b1000, true, 0, 5); // Reset matte flag 1, plane A visible.
    renderer.m_matteControl[6] = makeCommand(0b1000, true, 0, 5); // Not above previous command, must not infinite loop.
    renderer.m_matteControl[7] = makeCommand(0b0100, true, 0, 740); // ICF A 0, must be ignored.
}

static constexpr void configureTwoMattesLower(Video::Renderer& renderer) noexcept
{
    renderer.m_transparencyControl[PLANEA] = 0b1100; // Matte flag 1 = false.
    renderer.m_transparencyControl[PLANEB] = 0b1011; // Matte flag 0 = false.
    renderer.m_mix = false;
    renderer.m_planeOrder = true; // B in front of A.

    renderer.m_matteNumber = true;
    renderer.m_icf[PLANEA] = 63;
    renderer.m_icf[PLANEB] = 63;
    // starts with matte flags false.
    // mf bit must be ignored.
    renderer.m_matteControl[0] = makeCommand(0b1111, true, 31, 1); // Set matte flag 0, ICF B 31, plane B visible.
    renderer.m_matteControl[2] = makeCommand(0b1111, true, 0, 1); // Ignore later commands.
    renderer.m_matteControl[3] = makeCommand(0b1000, true, 0, 2); // ICF B 0, must be ignored.
    renderer.m_matteControl[3] = makeCommand(0, true, 63, 3); // ICF B 63, must be ignored.

    renderer.m_matteControl[4] = makeCommand(0b1101, false, 63, 2); // Set matte flag 1, ICF A 63, plane A visible.
    renderer.m_matteControl[5] = makeCommand(0b0100, false, 31, 4); // ICF A 31.
    renderer.m_matteControl[6] = makeCommand(0b0100, false, 63, 4); // Ignored.
    renderer.m_matteControl[7] = makeCommand(0b0110, false, 63, 8); // ICF B 63, ignored.
}

TEST_CASE("Matte lower position", "[Video]")
{
    // Tests the case where a next matte control register has a lower position than the next one.
    Video::RendererSoftware rendererSoft;
    IF_SIMD(Video::RendererSIMD rendererSIMD)

    SECTION("One matte")
    {
        configureOneMatteLower(rendererSoft);
        IF_SIMD(configureOneMatteLower(rendererSIMD))

        configureCLUT(rendererSoft);
        IF_SIMD(configureCLUT(rendererSIMD))

        rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        IF_SIMD(rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0))

        constexpr std::array<Video::Pixel, 768> EXPECTED_A = [] () {
            std::array<Video::Pixel, 768> array{RED, RED, RED, HIDDEN_RED, HIDDEN_RED};
            std::fill(array.begin() + 5, array.end(), RED);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_B = [] () {
            std::array<Video::Pixel, 768> array;
            std::ranges::fill(array, GREEN);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_SCREEN = [] () {
            std::array<Video::Pixel, 768> array{
                RED, RED, HALF_RED, GREEN, HALF_GREEN,
            };
            std::fill(array.begin() + 5, array.end(), HALF_RED);
            return array;
        }();

        REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSoft.m_plane[PLANEA].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSIMD.m_plane[PLANEA].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSoft.m_plane[PLANEB].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSIMD.m_plane[PLANEB].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSoft.m_screen.GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSIMD.m_screen.GetLinePointer(0))))
    }

    SECTION("Two matte")
    {
        configureTwoMattesLower(rendererSoft);
        IF_SIMD(configureTwoMattesLower(rendererSIMD))

        configureCLUT(rendererSoft);
        IF_SIMD(configureCLUT(rendererSIMD))

        rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        IF_SIMD(rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0))

        constexpr std::array<Video::Pixel, 768> EXPECTED_A = [] () {
            std::array<Video::Pixel, 768> array{
                HIDDEN_RED, HIDDEN_RED,
            };
            std::fill(array.begin() + 2, array.end(), RED);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_B = [] () {
            std::array<Video::Pixel, 768> array{HIDDEN_GREEN};
            std::fill(array.begin() + 1, array.end(), GREEN);
            return array;
        }();
        constexpr std::array<Video::Pixel, 768> EXPECTED_SCREEN = [] () {
            std::array<Video::Pixel, 768> array{BLUE};
            std::fill(array.begin() + 1, array.end(), HALF_GREEN);
            return array;
        }();

        REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSoft.m_plane[PLANEA].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_A.cbegin(), EXPECTED_A.cend(), rendererSIMD.m_plane[PLANEA].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSoft.m_plane[PLANEB].GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_B.cbegin(), EXPECTED_B.cend(), rendererSIMD.m_plane[PLANEB].GetLinePointer(0))))

        REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSoft.m_screen.GetLinePointer(0)));
        IF_SIMD(REQUIRE(std::equal(EXPECTED_SCREEN.cbegin(), EXPECTED_SCREEN.cend(), rendererSIMD.m_screen.GetLinePointer(0))))
    }
}

TEST_CASE("Decoding length", "[Video]")
{
    Video::RendererSoftware rendererSoft;
    IF_SIMD(Video::RendererSIMD rendererSIMD)

    SECTION("RGB555")
    {
        constexpr std::array<uint8_t, 384> INPUT_A{};
        constexpr std::array<uint8_t, 384> INPUT_B{};

        constexpr auto configure = [] (Video::Renderer& renderer) {
            renderer.m_imageType[PLANEA] = Video::Renderer::ImageType::Normal;
            renderer.m_imageType[PLANEB] = Video::Renderer::ImageType::Normal;
            renderer.m_codingMethod[PLANEA] = ICM(OFF);
            renderer.m_codingMethod[PLANEB] = ICM(RGB555);
            renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::PAL, false, false);
        };

        configure(rendererSoft);
        IF_SIMD(configure(rendererSIMD))

        std::pair<uint16_t, uint16_t> resSoft = rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSoft == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#if LIBCEDIMU_ENABLE_RENDERERSIMD
        std::pair<uint16_t, uint16_t> resSIMD = rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSIMD == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#endif
    }

    SECTION("DYUV")
    {
        constexpr std::array<uint8_t, 720> INPUT_A{};
        constexpr std::array<uint8_t, 720> INPUT_B{};

        constexpr auto configure = [] (Video::Renderer& renderer) {
            renderer.m_imageType[PLANEA] = Video::Renderer::ImageType::Normal;
            renderer.m_imageType[PLANEB] = Video::Renderer::ImageType::Normal;
            renderer.m_codingMethod[PLANEA] = ICM(DYUV);
            renderer.m_codingMethod[PLANEB] = ICM(DYUV);
            renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::NTSCMonitor, true, true);
        };

        configure(rendererSoft);
        IF_SIMD(configure(rendererSIMD))

        std::pair<uint16_t, uint16_t> resSoft = rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSoft == std::make_pair(INPUT_A.size() / 2, INPUT_B.size() / 2)); // Pixels are duplicated in high res DYUV.
#if LIBCEDIMU_ENABLE_RENDERERSIMD
        std::pair<uint16_t, uint16_t> resSIMD = rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSIMD == std::make_pair(INPUT_A.size() / 2, INPUT_B.size() / 2));
#endif
    }

    SECTION("CLUT7")
    {
        constexpr std::array<uint8_t, 384> INPUT_A{};
        constexpr std::array<uint8_t, 384> INPUT_B{};

        constexpr auto configure = [] (Video::Renderer& renderer) {
            renderer.m_imageType[PLANEA] = Video::Renderer::ImageType::Normal;
            renderer.m_imageType[PLANEB] = Video::Renderer::ImageType::Normal;
            renderer.m_codingMethod[PLANEA] = ICM(CLUT7);
            renderer.m_codingMethod[PLANEB] = ICM(CLUT7);
            renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::NTSCTV, false, true);
        };

        configure(rendererSoft);
        IF_SIMD(configure(rendererSIMD))

        std::pair<uint16_t, uint16_t> resSoft = rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSoft == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#if LIBCEDIMU_ENABLE_RENDERERSIMD
        std::pair<uint16_t, uint16_t> resSIMD = rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSIMD == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#endif
    }

    SECTION("CLUT4")
    {
        constexpr std::array<uint8_t, 720> INPUT_A{};
        constexpr std::array<uint8_t, 720> INPUT_B{};

        constexpr auto configure = [] (Video::Renderer& renderer) {
            renderer.m_imageType[PLANEA] = Video::Renderer::ImageType::Normal;
            renderer.m_imageType[PLANEB] = Video::Renderer::ImageType::Normal;
            renderer.m_codingMethod[PLANEA] = ICM(CLUT4);
            renderer.m_codingMethod[PLANEB] = ICM(CLUT4);
            renderer.SetDisplayFormat(Video::Renderer::DisplayFormat::NTSCMonitor, false, true);
        };

        configure(rendererSoft);
        IF_SIMD(configure(rendererSIMD))

        std::pair<uint16_t, uint16_t> resSoft = rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSoft == std::make_pair(INPUT_A.size() / 2, INPUT_B.size() / 2)); // Two pixels per byte.
#if LIBCEDIMU_ENABLE_RENDERERSIMD
        std::pair<uint16_t, uint16_t> resSIMD = rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSIMD == std::make_pair(INPUT_A.size() / 2, INPUT_B.size() / 2));
#endif
    }

    SECTION("Run-length")
    {
        constexpr std::array<uint8_t, 2> INPUT_A{0x80, 0};
        constexpr std::array<uint8_t, 5> INPUT_B{0x00, 0x80, 2, 0x80, 0};

        constexpr auto configure = [] (Video::Renderer& renderer) {
            renderer.m_imageType[PLANEA] = Video::Renderer::ImageType::RunLength;
            renderer.m_imageType[PLANEB] = Video::Renderer::ImageType::RunLength;
            renderer.m_codingMethod[PLANEA] = ICM(CLUT7);
            renderer.m_codingMethod[PLANEB] = ICM(CLUT7);
        };

        configure(rendererSoft);
        IF_SIMD(configure(rendererSIMD))

        std::pair<uint16_t, uint16_t> resSoft = rendererSoft.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSoft == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#if LIBCEDIMU_ENABLE_RENDERERSIMD
        std::pair<uint16_t, uint16_t> resSIMD = rendererSIMD.DrawLine(INPUT_A.data(), INPUT_B.data(), 0);
        REQUIRE(resSIMD == std::make_pair(INPUT_A.size(), INPUT_B.size()));
#endif
    }
}
