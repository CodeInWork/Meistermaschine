#ifndef VOLUME_SERVICE_H
#define VOLUME_SERVICE_H

#include <Arduino.h>

namespace VolumePins {
    constexpr uint8_t POT = A0;
}

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