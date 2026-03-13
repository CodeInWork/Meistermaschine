#ifndef MCP23017_DRIVER_H
#define MCP23017_DRIVER_H

#include <Arduino.h>
#include <Wire.h>

enum class MCPPin : uint8_t {
    GPA0 = 0,
    GPA1 = 1,
    GPA2 = 2,
    GPA3 = 3,
    GPA4 = 4,
    GPA5 = 5,
    GPA6 = 6,
    GPA7 = 7,

    GPB0 = 8,
    GPB1 = 9,
    GPB2 = 10,
    GPB3 = 11,
    GPB4 = 12,
    GPB5 = 13,
    GPB6 = 14,
    GPB7 = 15
};

namespace MCP {
    constexpr uint8_t DEFAULT_ADDRESS = 0x20;

    // BANK = 0
    constexpr uint8_t IODIRA = 0x00;
    constexpr uint8_t IODIRB = 0x01;
    constexpr uint8_t GPPUA  = 0x0C;
    constexpr uint8_t GPPUB  = 0x0D;
    constexpr uint8_t GPIOA  = 0x12;
    constexpr uint8_t GPIOB  = 0x13;
    constexpr uint8_t OLATA  = 0x14;
    constexpr uint8_t OLATB  = 0x15;
    
    constexpr uint16_t bitMask(MCPPin pin) noexcept
    {
        return static_cast<uint16_t>(1u << static_cast<uint8_t>(pin));
    }

    constexpr uint8_t lowByte16(uint16_t value) noexcept    //name conflict with Arduino macro, hence the 16 suffix
    {
        return static_cast<uint8_t>(value & 0x00FFu);       //name conflict with Arduino macro, hence the 16 suffix
    }

    constexpr uint8_t highByte16(uint16_t value) noexcept {
        return static_cast<uint8_t>((value >> 8) & 0x00FFu);
    }

    constexpr bool bitIsSet(uint16_t value, MCPPin pin) noexcept {
        return (value & bitMask(pin)) != 0;
    }
}

class MCP23017Driver {
public:
    explicit MCP23017Driver(uint8_t address = MCP::DEFAULT_ADDRESS);

    void begin();

    bool writeRegister(uint8_t reg, uint8_t value);
    bool readRegister(uint8_t reg, uint8_t& value);

    bool writeRegisterPair(uint8_t firstReg, uint16_t value);
    bool readRegisterPair(uint8_t firstReg, uint16_t& value);

    bool setDirection(uint16_t directionMask);
    bool setPullups(uint16_t pullupMask);

    bool readGPIO(uint16_t& value);
    bool writeGPIO(uint16_t value);

    bool writeOutputsLatched(uint16_t value);
    bool readOutputsLatched(uint16_t& value);

    bool setOutputState(uint16_t value);
    uint16_t outputState() const;

    bool setPinHigh(MCPPin pin);
    bool setPinLow(MCPPin pin);
    bool writePin(MCPPin pin, bool level);

private:
    uint8_t _address;
    uint16_t _outputState;
};

#endif