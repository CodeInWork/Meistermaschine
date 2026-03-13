#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class DisplayService
{
public:
    DisplayService();

    void begin();
    void showMessage(const char* text);
    void showTrackName(const char* name);

private:
    LiquidCrystal_I2C _lcd;
};

#endif