#include "MCP23017Driver.h"

MCP23017Driver::MCP23017Driver(uint8_t address)
    : _address(address),
      _outputState(0) {}

void MCP23017Driver::begin() {
    Wire.begin();
}

bool MCP23017Driver::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);
    Wire.write(value);
    return Wire.endTransmission() == 0;
}

bool MCP23017Driver::readRegister(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(_address);
    Wire.write(reg);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(_address, static_cast<uint8_t>(1)) != 1) {
        return false;
    }

    value = Wire.read();
    return true;
}

bool MCP23017Driver::writeRegisterPair(uint8_t firstReg, uint16_t value) {
    Wire.beginTransmission(_address);
    Wire.write(firstReg);
    Wire.write(MCP::lowByte16(value));
    Wire.write(MCP::highByte16(value));
    return Wire.endTransmission() == 0;
}

bool MCP23017Driver::readRegisterPair(uint8_t firstReg, uint16_t& value) {
    Wire.beginTransmission(_address);
    Wire.write(firstReg);

    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    if (Wire.requestFrom(_address, static_cast<uint8_t>(2)) != 2) {
        return false;
    }

    const uint8_t low  = Wire.read();
    const uint8_t high = Wire.read();

    value = static_cast<uint16_t>(low)
          | (static_cast<uint16_t>(high) << 8);

    return true;
}

bool MCP23017Driver::setDirection(uint16_t directionMask) {
    return writeRegisterPair(MCP::IODIRA, directionMask);
}

bool MCP23017Driver::setPullups(uint16_t pullupMask) {
    return writeRegisterPair(MCP::GPPUA, pullupMask);
}

bool MCP23017Driver::readGPIO(uint16_t& value) {
    return readRegisterPair(MCP::GPIOA, value);
}

bool MCP23017Driver::writeGPIO(uint16_t value) {
    return writeRegisterPair(MCP::GPIOA, value);
}

bool MCP23017Driver::writeOutputsLatched(uint16_t value) {
    return writeRegisterPair(MCP::OLATA, value);
}

bool MCP23017Driver::readOutputsLatched(uint16_t& value) {
    return readRegisterPair(MCP::OLATA, value);
}

bool MCP23017Driver::setOutputState(uint16_t value) {
    _outputState = value;
    return writeOutputsLatched(_outputState);
}

uint16_t MCP23017Driver::outputState() const {
    return _outputState;
}

bool MCP23017Driver::setPinHigh(uint8_t pin) {
    if (pin > 15) {
        return false;
    }

    _outputState |= MCP::bitMask(pin);
    return writeOutputsLatched(_outputState);
}

bool MCP23017Driver::setPinLow(uint8_t pin) {
    if (pin > 15) {
        return false;
    }

    _outputState &= static_cast<uint16_t>(~MCP::bitMask(pin));
    return writeOutputsLatched(_outputState);
}

bool MCP23017Driver::writePin(uint8_t pin, bool level) {
    return level ? setPinHigh(pin) : setPinLow(pin);
}