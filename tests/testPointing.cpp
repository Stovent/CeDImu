#include <catch2/catch_test_macros.hpp>

#include "Pointing.hpp"

#include <array>

TEST_CASE("Maneuvering", "[Pointing]")
{
    Pointing::Maneuvering maneuvering{Pointing::Maneuvering::GamepadSpeed::I};

    SECTION("Packet")
    {
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == false); // No button pressed, no packet.

        maneuvering.SetButton1(true);

        REQUIRE(maneuvering.IncrementTime(1.0) == true); // Now a packet.
        std::array<uint8_t, 4> packet{};
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x60);
        REQUIRE(packet[1] == 0x00);
        REQUIRE(packet[2] == 0x00);

        maneuvering.SetButton1(false);
        maneuvering.SetButton2(true);
        maneuvering.SetUp(true);
        maneuvering.SetRight(true);

        REQUIRE(maneuvering.IncrementTime(24'999'999.0) == false); // Not a packet yet.
        REQUIRE(maneuvering.IncrementTime(1.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x5C);
        REQUIRE(packet[1] == 0x01);
        REQUIRE(packet[2] == 0x3F);
    }

    SECTION("Acceleration")
    {
        maneuvering.SetSpeed(Pointing::Maneuvering::GamepadSpeed::II);

        maneuvering.SetDown(true);
        maneuvering.SetLeft(true);
        std::array<uint8_t, 4> packet{};

        // 2
        REQUIRE(maneuvering.IncrementTime(1.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x3E);
        REQUIRE(packet[2] == 0x02);

        // 4
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x3C);
        REQUIRE(packet[2] == 0x04);

        // 6
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x3A);
        REQUIRE(packet[2] == 0x06);

        // 8
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x38);
        REQUIRE(packet[2] == 0x08);

        // 10
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x36);
        REQUIRE(packet[2] == 0x0A);

        // 12
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x34);
        REQUIRE(packet[2] == 0x0C);

        // 14
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x32);
        REQUIRE(packet[2] == 0x0E);

        // 16
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x30);
        REQUIRE(packet[2] == 0x10);

        // 16
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x30);
        REQUIRE(packet[2] == 0x10);

        // Reset acceleration
        maneuvering.SetDown(false);
        maneuvering.SetLeft(false);

        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == false);
        REQUIRE(maneuvering.GetState(packet) == 3);

        REQUIRE(packet[0] == 0x40);
        REQUIRE(packet[1] == 0x00);
        REQUIRE(packet[2] == 0x00);

        // 2
        maneuvering.SetDown(true);
        maneuvering.SetLeft(true);

        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true); // Now a packet.
        REQUIRE(maneuvering.GetState(packet) == 3); // 3-bytes packet in maneuvering device.

        REQUIRE(packet[0] == 0x43);
        REQUIRE(packet[1] == 0x3E);
        REQUIRE(packet[2] == 0x02);
    }

    SECTION("No acceleration")
    {
        // If only buttons are pressed, this should not increase acceleration.
        maneuvering.SetSpeed(Pointing::Maneuvering::GamepadSpeed::II);

        std::array<uint8_t, 4> packet{};

        maneuvering.SetButton1(true);
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true);
        maneuvering.SetButton2(true);
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true);

        maneuvering.SetRight(true);
        REQUIRE(maneuvering.IncrementTime(25'000'000.0) == true);
        REQUIRE(maneuvering.GetState(packet) == 3);

        REQUIRE(packet[1] == 2); // First packet with direction: acceleration just started.
    }
}
