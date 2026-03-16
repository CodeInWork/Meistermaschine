#ifndef BUTTON_UTILS_H
#define BUTTON_UTILS_H

#include <Arduino.h>

namespace ButtonLayout
{
    static constexpr uint8_t COLUMN_COUNT = 4;
    static constexpr uint8_t ROW_COUNT = 5;
    static constexpr uint8_t BUTTON_COUNT = COLUMN_COUNT * ROW_COUNT;
    static constexpr uint8_t NO_BUTTON = 255;

    using ButtonId = uint8_t;
    using ButtonMask = uint32_t;

    constexpr ButtonId toButtonId(uint8_t column, uint8_t row) noexcept
    {
        return static_cast<ButtonId>(column * ROW_COUNT + row);
    }

    constexpr uint8_t columnOf(ButtonId buttonId) noexcept
    {
        return static_cast<uint8_t>(buttonId / ROW_COUNT);
    }

    constexpr uint8_t rowOf(ButtonId buttonId) noexcept
    {
        return static_cast<uint8_t>(buttonId % ROW_COUNT);
    }

    constexpr ButtonMask ID2Mask(ButtonId buttonId) noexcept
    {
        return static_cast<ButtonMask>(1UL << buttonId);
    }

    constexpr bool isValid(ButtonId buttonId) noexcept
    {
        return buttonId < BUTTON_COUNT;
    }

    inline bool singleSetBitToButtonId(ButtonMask mask, ButtonId& buttonId) noexcept
    {
        if (mask == 0) {
            return false;
        }

        if ((mask & (mask - 1)) != 0) {
            return false;
        }

        buttonId = static_cast<ButtonId>(
            __builtin_ctzl(static_cast<unsigned long>(mask))
        );

        return isValid(buttonId);
    }

    inline ButtonMask lowestSetBit(ButtonMask mask) noexcept
    {
        return mask & static_cast<ButtonMask>(-static_cast<int32_t>(mask));
    }
}

#endif