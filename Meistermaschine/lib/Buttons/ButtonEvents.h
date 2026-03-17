#ifndef BUTTON_EVENTS_H
#define BUTTON_EVENTS_H

#include <Arduino.h>
#include "ButtonUtils.h"

struct ButtonEvents
{
    ButtonLayout::ButtonMask pressed = 0;
    ButtonLayout::ButtonMask released = 0;
    ButtonLayout::ButtonMask held = 0;
    bool valid = false;
};

#endif