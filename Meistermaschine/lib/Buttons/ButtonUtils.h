#ifndef BUTTON_UTILS_H
#define BUTTON_UTILS_H

#include <Arduino.h>

inline bool singleSetBitToButtonId(uint32_t mask, uint8_t& buttonId)
{
    if (mask == 0) return false;

    if ((mask & (mask - 1)) != 0)
        return false;

    buttonId = static_cast<uint8_t>(__builtin_ctzl(static_cast<unsigned long>(mask)));      // count trailing zeros
    return true;
}

inline uint8_t buttonColumn(uint8_t buttonId)
{
    return buttonId / 5;
}

inline uint8_t buttonRow(uint8_t buttonId)
{
    return buttonId % 5;
}

#endif