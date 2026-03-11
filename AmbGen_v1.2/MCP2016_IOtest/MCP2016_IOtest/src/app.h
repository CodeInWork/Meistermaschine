#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include "MCP23017Driver.h"
#include "MCP23017Buttons.h"

namespace AppPins {
    constexpr uint16_t BTN_0 = Buttons::BTN_0;
    constexpr uint16_t BTN_1 = Buttons::BTN_1;
    constexpr uint16_t LED_0 = MCP::bitMask(MCPPin::GPB0);
    constexpr uint16_t LED_1 = MCP::bitMask(MCPPin::GPB1);
}

enum class Led1State : uint8_t
{
    Off,
    On,
    Blinking
};

class App {
public:
    App(MCP23017Driver& driver, MCP23017Buttons& buttons);

    bool begin();
    void update(uint32_t now);

private:
    void handleButtonEvents(const ButtonEvents& ev);
    void updateLed1State(uint32_t now);
    void updateOutputs();

    void setLed(uint16_t ledMask, bool on);
    void toggleLed(uint16_t ledMask);

private:
    MCP23017Driver& _driver;
    MCP23017Buttons& _buttons;

    uint16_t _ledState;
    bool _outputsDirty;

    Led1State _led1State;
    uint32_t _lastBlinkMs;

    static constexpr uint32_t BLINK_INTERVAL_MS = 250;
};

#endif