#ifndef VOLUME_SERVICE_H
#define VOLUME_SERVICE_H

#include <Arduino.h>
#include "HardwareConfig.h"

class VolumeService
{
public:
    VolumeService();

    void begin();
    bool readIfChanged(uint8_t& volumeOut);

private:
    uint8_t _lastVolume;
};

#endif