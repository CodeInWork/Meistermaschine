#include "DisplayService.h"

DisplayService::DisplayService()
    : _lcd(0x27, 16, 2)
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
    _lcd.clear();
    _lcd.setCursor(0, 0);
    _lcd.print(name);
}