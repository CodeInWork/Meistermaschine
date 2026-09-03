#ifndef BUTTON_EVENTS_H
#define BUTTON_EVENTS_H

#include <Arduino.h>
#include "ButtonUtils.h"

struct ButtonEvents
{
    ButtonLayout::Coord pressed;
    ButtonLayout::Coord released;
    ButtonLayout::Coord current;
    ButtonLayout::Coord longPressed;
};

#endif



