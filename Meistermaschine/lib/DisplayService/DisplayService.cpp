#include "DisplayService.h"
#include "HardwareConfig.h"

DisplayService::DisplayService()
    : _lcd(I2CAddresses::LCD, 16, 2)
{
}

void DisplayService::begin()
{
    _lcd.init();
    _lcd.backlight();
}

void DisplayService::showMessage(const char* text)
{
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(text);
}

void DisplayService::showTrackName(const char* name)
{
    const char* title = extractTitle(name);
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(title);
}

const char* DisplayService::extractTitle(const char* path) const
{
    if (path == nullptr) {
        return "";
    }

    const char* fileName = path;

    for (const char* p = path; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            fileName = p + 1;
        }
    }

    return fileName;
}

void DisplayService::showLayout(
    const char* top,
    const char* bottomLeft,
    const char* bottomRight
)
{
    _lcd.clear();

    // --- Top row ---
    _lcd.setCursor(0, 0);

    for (uint8_t i = 0; i < 16 && top[i] != '\0'; ++i) {
        _lcd.print(top[i]);
    }

    // --- Bottom left ---
    const size_t rightLength =
        min(strlen(bottomRight), static_cast<size_t>(16));

    const uint8_t rightStart =
        static_cast<uint8_t>(16 - rightLength);

    _lcd.setCursor(0, 1);

    for (
        uint8_t i = 0;
        i < rightStart &&
        bottomLeft[i] != '\0';
        ++i
    ) {
        _lcd.print(bottomLeft[i]);
    }

    // --- Bottom right ---
    _lcd.setCursor(rightStart, 1);

    for (
        size_t i = 0;
        i < rightLength;
        ++i
    ) {
        _lcd.print(bottomRight[i]);
    }
}