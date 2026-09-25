#ifndef SOFTWARE_CONFIG_H
#define SOFTWARE_CONFIG_H

#include "ButtonUtils.h"

namespace SoftwareConfig
{
constexpr bool DEBUG = false;

constexpr bool DEFAULT_LOOP_CONFIG[
    ButtonLayout::COLUMN_COUNT
] = {
    true,
    true,
    true,
    false
};
}

#endif // SOFTWARE_CONFIG_H