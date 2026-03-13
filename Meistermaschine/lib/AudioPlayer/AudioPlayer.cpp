#include "TrackLibrary.h"
#include "AudioPlayer.h"

TrackLibrary::TrackLibrary()
    : _initialized(false)
{
}

bool TrackLibrary::begin()
{
    // Alle SPI-Teilnehmer sicher deselektieren
    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    if (!SD.begin(AudioPins::CARDCS)) {
        Serial.println(F("SD failed, or not present"));
        return false;
    }

    File registry = SD.open(REGISTRY_FILE);
    if (!registry) {
        Serial.println(F("Registry file not found"));
        return false;
    }

    registry.close();
    _initialized = true;

    Serial.println(F("TrackLibrary ready"));
    return true;
}

bool TrackLibrary::loadPlaylist(uint8_t requestedButtonId, Playlist& playlist) const
{
    clearPlaylist(playlist);
    playlist.buttonId = requestedButtonId;

    if (!_initialized) {
        return false;
    }

    File registry = SD.open(REGISTRY_FILE);
    if (!registry) {
        Serial.println(F("Failed to open registry"));
        return false;
    }

    char line[MAX_LINE_LEN] = {0};
    uint8_t linePos = 0;

    while (registry.available()) {
        const char c = static_cast<char>(registry.read());

        // Zeilenende behandeln
        if (c == '\n' || c == '\r') {
            if (linePos > 0) {
                line[linePos] = '\0';

                uint8_t parsedButtonId = 0;
                char fileName[MAX_FILENAME_LEN] = {0};

                if (parseLine(line, parsedButtonId, fileName, sizeof(fileName))) {
                    if (parsedButtonId == requestedButtonId) {
                        if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
                            strncpy(playlist.tracks[playlist.trackCount], fileName, MAX_FILENAME_LEN - 1);
                            playlist.tracks[playlist.trackCount][MAX_FILENAME_LEN - 1] = '\0';
                            ++playlist.trackCount;
                        } else {
                            Serial.println(F("Playlist full, additional tracks ignored"));
                        }
                    }
                }

                linePos = 0;
            }

            continue;
        }

        if (linePos < (MAX_LINE_LEN - 1)) {
            line[linePos++] = c;
        }
    }

    // Letzte Zeile verarbeiten, falls Datei nicht mit newline endet
    if (linePos > 0) {
        line[linePos] = '\0';

        uint8_t parsedButtonId = 0;
        char fileName[MAX_FILENAME_LEN] = {0};

        if (parseLine(line, parsedButtonId, fileName, sizeof(fileName))) {
            if (parsedButtonId == requestedButtonId) {
                if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
                    strncpy(playlist.tracks[playlist.trackCount], fileName, MAX_FILENAME_LEN - 1);
                    playlist.tracks[playlist.trackCount][MAX_FILENAME_LEN - 1] = '\0';
                    ++playlist.trackCount;
                }
            }
        }
    }

    registry.close();

    return (playlist.trackCount > 0);
}

bool TrackLibrary::hasTrack(uint8_t buttonId) const
{
    Playlist playlist;
    return loadPlaylist(buttonId, playlist);
}

bool TrackLibrary::parseLine(const char* line, uint8_t& buttonId, char* fileName, size_t fileNameSize) const
{
    if (!line || !fileName || fileNameSize == 0) {
        return false;
    }

    // Minimalformat: "00 a.mp3"
    if (!isDigit(line[0]) || !isDigit(line[1])) {
        return false;
    }

    // dritte Position sollte Trennzeichen sein
    if (line[2] != ' ' && line[2] != '\t') {
        return false;
    }

    buttonId = static_cast<uint8_t>((line[0] - '0') * 10 + (line[1] - '0'));

    // Trenner überspringen
    uint8_t i = 2;
    while (line[i] == ' ' || line[i] == '\t') {
        ++i;
    }

    if (line[i] == '\0') {
        return false;
    }

    strncpy(fileName, &line[i], fileNameSize - 1);
    fileName[fileNameSize - 1] = '\0';

    return true;
}

bool TrackLibrary::isDigit(char c) const
{
    return (c >= '0' && c <= '9');
}

void TrackLibrary::clearPlaylist(Playlist& playlist) const
{
    playlist.buttonId = 0;
    playlist.trackCount = 0;

    for (uint8_t i = 0; i < MAX_PLAYLIST_TRACKS; ++i) {
        playlist.tracks[i][0] = '\0';
    }
}