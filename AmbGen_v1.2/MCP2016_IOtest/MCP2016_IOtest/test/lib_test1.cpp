#include <Arduino.h>
#include "MCP23017Driver.h"
#include "MCP23017Buttons.h"

// --- Hardware-Instanzen ---
MCP23017Driver mcp(0x20);
MCP23017Buttons buttons(mcp, 5, 20);

// --- Pin-Masken für LEDs ---
namespace Outputs {
    constexpr uint16_t LED_0 = MCP::bitMask(MCP::GPB0);
    constexpr uint16_t LED_1 = MCP::bitMask(MCP::GPB1);
}

// --- Pin-Masken für Taster ---
namespace Inputs {
    constexpr uint16_t BTN_0 = Buttons::BTN_0;   // GPA0
    constexpr uint16_t BTN_1 = Buttons::BTN_1;   // GPA1
}

// --- LED-Zustand im RAM ---
uint16_t ledState = 0;

// --- Blink-Logik für LED_1 ---
bool led1BlinkEnabled = false;
bool led1BlinkPhase = false;
uint32_t lastBlinkMs = 0;
constexpr uint32_t BLINK_INTERVAL_MS = 250;

// --------------------------------------------------
// Hilfsfunktionen für LEDs
// --------------------------------------------------

void setLed(uint16_t ledMask, bool on)
{
    if (on) {
        ledState |= ledMask;
    } else {
        ledState &= static_cast<uint16_t>(~ledMask);
    }
}

void toggleLed(uint16_t ledMask)
{
    ledState ^= ledMask;
}

bool applyOutputs()
{
    return mcp.setOutputState(ledState);
}

// --------------------------------------------------
// Initialisierung
// --------------------------------------------------

bool initHardware()
{
    mcp.begin();

    // Buttons initialisieren:
    // BTN_0 und BTN_1 aktiv, Pull-Ups an
    const uint16_t buttonMask = Inputs::BTN_0 | Inputs::BTN_1;
    if (!buttons.begin(buttonMask)) {
        return false;
    }

    // Richtung komplett setzen:
    // 1 = Input, 0 = Output
    //
    // BTN_0 / BTN_1 bleiben Inputs
    // LED_0 / LED_1 werden Outputs
    //
    // Wir setzen zunächst alles auf Input und ziehen dann die LED-Bits auf 0.
    uint16_t directionMask = 0xFFFF;
    directionMask &= static_cast<uint16_t>(~Outputs::LED_0);
    directionMask &= static_cast<uint16_t>(~Outputs::LED_1);

    if (!mcp.setDirection(directionMask)) {
        return false;
    }

    // Pull-Ups nur für Tasterpins aktiv
    if (!mcp.setPullups(buttonMask)) {
        return false;
    }

    ledState = 0;
    if (!applyOutputs()) {
        return false;
    }

    return true;
}

// --------------------------------------------------
// Tasterlogik
// --------------------------------------------------

void handleButtonEvents(const ButtonEvents& ev)
{
    if (ev.pressed & Inputs::BTN_0) {
        toggleLed(Outputs::LED_0);
        Serial.println("BTN_0 pressed -> toggle LED_0");
    }

    if (ev.pressed & Inputs::BTN_1) {
        led1BlinkEnabled = !led1BlinkEnabled;
        Serial.println("BTN_1 pressed -> toggle LED_1 blink mode");

        if (!led1BlinkEnabled) {
            led1BlinkPhase = false;
            setLed(Outputs::LED_1, false);
        }
    }
}

// --------------------------------------------------
// Nicht blockierende Blink-Logik
// --------------------------------------------------

void updateBlinkTask(uint32_t now)
{
    if (!led1BlinkEnabled) {
        return;
    }

    if ((now - lastBlinkMs) >= BLINK_INTERVAL_MS) {
        lastBlinkMs = now;
        led1BlinkPhase = !led1BlinkPhase;
        setLed(Outputs::LED_1, led1BlinkPhase);
    }
}

// --------------------------------------------------
// Debug-Ausgabe
// --------------------------------------------------

void printPressedHeldReleased(const ButtonEvents& ev)
{
    if (ev.pressed) {
        Serial.print("Pressed : 0x");
        Serial.println(ev.pressed, HEX);
    }

    if (ev.released) {
        Serial.print("Released: 0x");
        Serial.println(ev.released, HEX);
    }

    if (ev.held) {
        Serial.print("Held    : 0x");
        Serial.println(ev.held, HEX);
    }
}

// --------------------------------------------------
// Arduino setup/loop
// --------------------------------------------------

void setup()
{
    Serial.begin(9600);
    delay(300);
    Serial.println("Start");

    if (!initHardware()) {
        Serial.println("Hardware init failed");
        while (true) {
        }
    }

    Serial.println("Hardware ready");
}

void loop()
{
    const uint32_t now = millis();

    const ButtonEvents ev = buttons.update();

    if (ev.valid) {
        handleButtonEvents(ev);
        // optional:
        // printPressedHeldReleased(ev);
    }

    updateBlinkTask(now);

    if (!applyOutputs()) {
        Serial.println("Output write failed");
    }
}