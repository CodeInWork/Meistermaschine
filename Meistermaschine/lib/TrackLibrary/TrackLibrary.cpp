#include "TrackLibrary.h"
#include "AudioPlayer.h"

TrackLibrary::TrackLibrary()
    :   _initialized(false),
        _presetName{0},
        _presetPath{0},
        _registryPath{0}
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

    if (!findPreset()) {
        Serial.println(F("No valid preset found"));
        return false;
    }

    Serial.print(F("Preset directory: "));
    Serial.println(_presetPath);

    Serial.print(F("Registry: "));
    Serial.println(_registryPath);

    Serial.print(F("Registry: "));
    Serial.println(_registryPath);

    File registry = SD.open(_registryPath, FILE_READ);

    if (!registry) {
        Serial.println(F("Failed to open registry"));
        return false;
    }

    registry.close();

    _initialized = true;

    Serial.println(F("TrackLibrary ready"));
    return true;
}

bool TrackLibrary::hasMmsExtension(
    const char* fileName
) const
{
    if (fileName == nullptr) {
        return false;
    }

    const size_t length = strlen(fileName);

    if (length < 4) {
        return false;
    }

    const char* extension = &fileName[length - 4];

    return
        extension[0] == '.' &&
        (extension[1] == 'm' || extension[1] == 'M') &&
        (extension[2] == 'm' || extension[2] == 'M') &&
        (extension[3] == 's' || extension[3] == 'S');
}

bool TrackLibrary::buildTrackPath(
    const char* fileName,
    char* trackPath,
    size_t trackPathSize
) const
{
    if (
        fileName == nullptr ||
        trackPath == nullptr ||
        trackPathSize == 0 ||
        _presetPath[0] == '\0'
    ) {
        return false;
    }

    const int written = snprintf(
        trackPath,
        trackPathSize,
        "%s/%s",
        _presetPath,
        fileName
    );

    if (
        written < 0 ||
        static_cast<size_t>(written) >= trackPathSize
    ) {
        trackPath[0] = '\0';
        return false;
    }

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

    File registry = SD.open(_registryPath, FILE_READ);

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

                    if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
                        char* trackPath =
                            playlist.tracks[playlist.trackCount];

                        if (
                            buildTrackPath(
                                fileName,
                                trackPath,
                                MAX_PATH_LEN
                            )
                        ) {
                            Serial.print(F("Constructed track path: "));
                            Serial.println(trackPath);

                            if (!SD.exists(trackPath)) {
                                Serial.print(F("Track file not found: "));
                                Serial.println(trackPath);
                            } else {
                                Serial.println(F("Track file exists"));
                            }

                            ++playlist.trackCount;
                        } else {
                            Serial.print(F("Could not build track path for: "));
                            Serial.println(fileName);
                        }
                    } else {
                        Serial.println(
                            F("Playlist full, additional tracks ignored")
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
    Serial.print(F("Matched button ["));
    Serial.print(parsedButton.column);
    Serial.print(F("]["));
    Serial.print(parsedButton.row);
    Serial.println(F("]"));

    if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
        if (
            buildTrackPath(
                fileName,
                playlist.tracks[playlist.trackCount],
                sizeof(playlist.tracks[playlist.trackCount])
            )
        ) {
            Serial.print(F("Track path: "));
            Serial.println(
                playlist.tracks[playlist.trackCount]
            );

            ++playlist.trackCount;
        } else {
            Serial.print(F("Track path too long: "));
            Serial.println(fileName);
        }
    } else {
        Serial.println(
            F("Playlist full, additional tracks ignored")
        );
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

const char* TrackLibrary::presetName() const
{
    return _presetName;
}

bool TrackLibrary::findPreset()
{
    _presetName[0] = '\0';
    _presetPath[0] = '\0';
    _registryPath[0] = '\0';

    File root = SD.open("/");

    if (!root) {
        Serial.println(F("Failed to open SD root"));
        return false;
    }

    while (true) {
        File directory = root.openNextFile();

        if (!directory) {
            break;
        }

        if (!directory.isDirectory()) {
            directory.close();
            continue;
        }

        const char* directoryName = directory.name();

        Serial.print(F("Checking directory: "));
        Serial.println(directoryName);

        while (true) {
            File entry = directory.openNextFile();

            if (!entry) {
                break;
            }

            Serial.print(F("  Entry: "));
            Serial.println(entry.name());

            if (
                !entry.isDirectory() &&
                hasMmsExtension(entry.name())
            ) {
                const size_t presetNameLength =
                    strlen(directoryName);

                if (
                    presetNameLength >
                    MAX_PRESET_NAME_CHARS
                ) {
                    Serial.println(F("Preset name too long"));

                    entry.close();
                    directory.close();
                    root.close();

                    return false;
                }

                strncpy(
                    _presetName,
                    directoryName,
                    MAX_PRESET_NAME_LEN - 1
                );

                _presetName[
                    MAX_PRESET_NAME_LEN - 1
                ] = '\0';

                const int presetWritten = snprintf(
                    _presetPath,
                    sizeof(_presetPath),
                    "/%s",
                    directoryName
                );

                const int registryWritten = snprintf(
                    _registryPath,
                    sizeof(_registryPath),
                    "%s/%s",
                    _presetPath,
                    entry.name()
                );

                entry.close();
                directory.close();
                root.close();

                if (
                    presetWritten < 0 ||
                    static_cast<size_t>(presetWritten) >=
                        sizeof(_presetPath)
                ) {
                    Serial.println(F("Preset path too long"));
                    return false;
                }

                if (
                    registryWritten < 0 ||
                    static_cast<size_t>(registryWritten) >=
                        sizeof(_registryPath)
                ) {
                    Serial.println(F("Registry path too long"));
                    return false;
                }

                Serial.print(F("Selected preset: "));
                Serial.println(_presetName);

                Serial.print(F("Registry path: "));
                Serial.println(_registryPath);

                return true;
            }

            entry.close();
        }

        directory.close();
    }

    root.close();

    Serial.println(F("No preset directory with MMS file found"));
    return false;
}