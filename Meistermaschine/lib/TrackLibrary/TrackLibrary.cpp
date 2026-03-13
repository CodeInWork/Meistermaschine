#include "TrackLibrary.h"
#include "AudioPlayer.h"

TrackLibrary::TrackLibrary()
    : _root()
{
}

bool TrackLibrary::begin()
{
    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    if (!SD.begin(AudioPins::CARDCS)) {
        return false;
    }

    _root = SD.open("/");
    return static_cast<bool>(_root);
}

uint8_t TrackLibrary::countDigits(uint8_t number) const
{
    if (number < 10) {
        return 1;
    }
    return 1 + countDigits(number / 10);
}

bool TrackLibrary::matchTrackPrefix(const char* fileName, uint8_t trackNumber) const
{
    char numberId[6] = {0};
    snprintf(numberId, sizeof(numberId), "%u_", trackNumber);
    return strncmp(fileName, numberId, strlen(numberId)) == 0;
}

bool TrackLibrary::getTrackFileName(uint8_t trackNumber, char* outName, size_t outSize)
{
    _root.rewindDirectory();

    while (true) {
        File entry = _root.openNextFile();
        if (!entry) {
            break;
        }

        if (!entry.isDirectory()) {
            const char* name = entry.name();
            if (matchTrackPrefix(name, trackNumber)) {
                strncpy(outName, name, outSize - 1);
                outName[outSize - 1] = '\0';
                entry.close();
                return true;
            }
        }

        entry.close();
    }

    return false;
}