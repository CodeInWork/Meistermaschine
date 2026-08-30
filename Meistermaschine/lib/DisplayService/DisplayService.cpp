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