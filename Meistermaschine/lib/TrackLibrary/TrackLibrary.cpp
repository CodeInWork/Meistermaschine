#include "TrackLibrary.h"
#include "AudioPlayer.h"

TrackLibrary::TrackLibrary()
    : _initialized(false)
{
}

bool TrackLibrary::begin()
{
    digitalWrite(AudioPins::CS, HIGH);
    digitalWrite(AudioPins::DCS, HIGH);
    digitalWrite(AudioPins::CARDCS, HIGH);

    if (!SD.begin(AudioPins::CARDCS)) {
        Serial.println(F("SD failed, or not present"));
        return false;
    }

    Serial.println(F("SD initialized"));

    Serial.print(F("Looking for: "));
    Serial.println(REGISTRY_FILE);

    Serial.print(F("SD.exists = "));
    Serial.println(SD.exists(REGISTRY_FILE));

    Serial.println(F("Testing registry open..."));

    File root = SD.open("/");
    Serial.println(F("Root listing:"));

    while (true) {
        File entry = root.openNextFile();
        if (!entry) {
            break;
        }

        Serial.println(entry.name());
        entry.close();
    }
    root.close();

    File registry = SD.open(REGISTRY_FILE, FILE_READ);

    Serial.print(F("open result = "));
    Serial.println(static_cast<bool>(registry));
    if (!registry) {
        Serial.println(F("Registry file not found"));
        return false;
    }

    Serial.println(F("Registry file found"));
    registry.close();

    _initialized = true;
    Serial.println(F("TrackLibrary ready"));
    return true;
}

bool TrackLibrary::loadPlaylist(
    const ButtonLayout::Coord& requestedButton,
    Playlist& playlist
) const
{
    clearPlaylist(playlist);
    playlist.button = requestedButton;

    if (!_initialized) {
        return false;
    }

    if (!ButtonLayout::isValid(requestedButton)) {
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
        const char c =
            static_cast<char>(registry.read());
        // treat end of line
        if (c == '\n' || c == '\r') {
            if (linePos > 0) {
                line[linePos] = '\0';

                ButtonLayout::Coord parsedButton;
                char fileName[MAX_FILENAME_LEN] = {0};

                if (
                    parseLine(
                        line,
                        parsedButton,
                        fileName,
                        sizeof(fileName)
                    ) &&
                    coordinatesEqual(
                        parsedButton,
                        requestedButton
                    )
                ) {
                    Serial.print(F("Matched button ["));
                    Serial.print(parsedButton.column);
                    Serial.print(F("]["));
                    Serial.print(parsedButton.row);
                    Serial.println(F("]"));

                    if (
                        playlist.trackCount <
                        MAX_PLAYLIST_TRACKS
                    ) {
                        strncpy(
                            playlist.tracks[
                                playlist.trackCount
                            ],
                            fileName,
                            MAX_FILENAME_LEN - 1
                        );

                        playlist.tracks[
                            playlist.trackCount
                        ][MAX_FILENAME_LEN - 1] = '\0';

                        ++playlist.trackCount;
                    } else {
                        Serial.println(
                            F(
                                "Playlist full, "
                                "additional tracks ignored"
                            )
                        );
                    }
                }

                linePos = 0;
            }

            continue;
        }

        if (linePos < MAX_LINE_LEN - 1) {
            line[linePos++] = c;
        }
    }

    // Process the final line if the file has no trailing newline.
    if (linePos > 0) {
        line[linePos] = '\0';

        ButtonLayout::Coord parsedButton;
        char fileName[MAX_FILENAME_LEN] = {0};

        if (
            parseLine(
                line,
                parsedButton,
                fileName,
                sizeof(fileName)
            ) &&
            coordinatesEqual(
                parsedButton,
                requestedButton
            )
        ) {
            if (
                playlist.trackCount <
                MAX_PLAYLIST_TRACKS
            ) {
                strncpy(
                    playlist.tracks[
                        playlist.trackCount
                    ],
                    fileName,
                    MAX_FILENAME_LEN - 1
                );

                playlist.tracks[
                    playlist.trackCount
                ][MAX_FILENAME_LEN - 1] = '\0';

                ++playlist.trackCount;
            }
        }
    }

    registry.close();

    return playlist.trackCount > 0;
}

bool TrackLibrary::hasTrack(
    const ButtonLayout::Coord& button
) const
{
    Playlist playlist;
    return loadPlaylist(button, playlist);
}

bool TrackLibrary::parseLine(
    const char* line,
    ButtonLayout::Coord& button,
    char* fileName,
    size_t fileNameSize
) const
{
    if (
        line == nullptr ||
        fileName == nullptr ||
        fileNameSize == 0
    ) {
        return false;
    }

    // Minimum format: "00 a.mp3"
    if (!isDigit(line[0]) || !isDigit(line[1])) {
        return false;
    }
    // Check for a space or tab after the two digits
    if (line[2] != ' ' && line[2] != '\t') {
        return false;
    }

    const uint8_t column =
        static_cast<uint8_t>(line[0] - '0');

    const uint8_t row =
        static_cast<uint8_t>(line[1] - '0');

    const ButtonLayout::Coord parsedButton{
        column,
        row
    };

    if (!ButtonLayout::isValid(parsedButton)) {
        return false;
    }

    button = parsedButton;

    uint8_t i = 2;

    while (line[i] == ' ' || line[i] == '\t') {
        ++i;
    }

    if (line[i] == '\0') {
        return false;
    }

    strncpy(
        fileName,
        &line[i],
        fileNameSize - 1
    );

    fileName[fileNameSize - 1] = '\0';

    return true;
}

bool TrackLibrary::coordinatesEqual(
    const ButtonLayout::Coord& first,
    const ButtonLayout::Coord& second
) const
{
    return
        first.column == second.column &&
        first.row == second.row;
}

bool TrackLibrary::isDigit(char c) const
{
    return (c >= '0' && c <= '9');
}

void TrackLibrary::clearPlaylist(
    Playlist& playlist
) const
{
    playlist.button = ButtonLayout::Coord{};
    playlist.trackCount = 0;

    for (
        uint8_t i = 0;
        i < MAX_PLAYLIST_TRACKS;
        ++i
    ) {
        playlist.tracks[i][0] = '\0';
    }
}