#include "VolumeService.h"

VolumeService::VolumeService()
    : _lastVolume(255)
{
}

void VolumeService::begin()
{
}

bool VolumeService::readIfChanged(uint8_t& volumeOut)
{
    const uint8_t newVolume =
        static_cast<uint8_t>(map(analogRead(VolumePins::POT), 0, 1023, 100, 0));

    if (newVolume == _lastVolume) {
        return false;
    }

    _lastVolume = newVolume;
    volumeOut = newVolume;
    return true;
}