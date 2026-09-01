#ifndef DISPLAY_SERVICE_H
#define DISPLAY_SERVICE_H

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class DisplayService
{
public:
    DisplayService();
    
    void showLayout(
        const char* top,
        const char* bottomLeft,
        const char* bottomRight
    );

    void begin();
    void showMessage(const char* text);
    void showTrackName(const char* name);

private:
    const char* extractTitle(const char* path) const;
private:
    LiquidCrystal_I2C _lcd;
};

#endif