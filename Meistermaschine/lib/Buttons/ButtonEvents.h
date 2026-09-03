#ifndef BUTTON_EVENTS_H
#define BUTTON_EVENTS_H

#include <Arduino.h>
#include "ButtonUtils.h"

struct ButtonEvents
{
    ButtonLayout::Coord held;
    ButtonLayout::Coord released;
    
    uint32_t pressDurationMs = 0;
};

#endif



