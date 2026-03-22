#ifndef HardwareConfig_H
#define HardwareConfig_H

#include <Arduino.h>

namespace I2CAddresses
{
    // MCP23017:
    // A2 A1 A0 = 0 0 0 → 0x20
    constexpr uint8_t MCP_1 = 0x20;

    // A2 A1 A0 = 0 0 1 → 0x21
    constexpr uint8_t MCP_2 = 0x21;

    // LCD (per I2C Scanner verifiziert)
    constexpr uint8_t LCD = 0x27;
}

namespace AudioPins
{
    constexpr uint8_t RESET  = 9;
    constexpr uint8_t CS     = 10;
    constexpr uint8_t DCS    = 8;
    constexpr uint8_t CARDCS = 4;
    constexpr uint8_t DREQ   = 3;
}

namespace LocalButtonPins
{
    constexpr uint8_t BTN_1 = 5;
    constexpr uint8_t BTN_2 = 6;
}

namespace VolumePins
{
    constexpr uint8_t POT = A0;
}

#endif