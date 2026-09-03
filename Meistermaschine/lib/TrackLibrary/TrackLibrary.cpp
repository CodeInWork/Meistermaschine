#include "TrackLibrary.h"
#include "AudioPlayer.h"
#include "SoftwareConfig.h"

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
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("SD failed, or not present"));
        }
        return false;
    }

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("SD initialized"));
    }

    if (!findPreset()) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("No valid preset found"));
        }
        return false;
    }

    if (SoftwareConfig::DEBUG) {
        Serial.print(F("Preset directory: "));
        Serial.println(_presetPath);

        Serial.print(F("Registry: "));
        Serial.println(_registryPath);

        Serial.print(F("Registry: "));
        Serial.println(_registryPath);
    }

    File registry = SD.open(_registryPath, FILE_READ);

    if (!registry) {
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Failed to open registry"));
        }
        return false;
    }

    registry.close();

    _initialized = true;

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("TrackLibrary ready"));
    }
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
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Failed to open registry"));
        }
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
                char title[MAX_TRACK_TITLE_LEN] = {0};

                if (
                    parseLine(
                        line,
                        parsedButton,
                        fileName,
                        sizeof(fileName),
                        title,
                        sizeof(title)
                    ) &&
                    coordinatesEqual(
                        parsedButton,
                        requestedButton
                    )
                ) {
                    if (SoftwareConfig::DEBUG) {
                        Serial.print(F("Matched button ["));
                        Serial.print(parsedButton.column);
                        Serial.print(F("]["));
                        Serial.print(parsedButton.row);
                        Serial.println(F("]"));
                    }

                    if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
                        char* trackPath =
                            playlist.tracks[playlist.trackCount];

                        if (
                            buildTrackPath(
                                fileName,
                                trackPath,
                                sizeof(playlist.tracks[playlist.trackCount])
                            )
                        ) {
                            if (SoftwareConfig::DEBUG) {
                                Serial.print(F("Constructed track path: "));
                                Serial.println(trackPath);

                                if (!SD.exists(trackPath)) {
                                    Serial.print(F("Track file not found: "));
                                    Serial.println(trackPath);
                                } else {
                                    Serial.println(F("Track file exists"));
                                }
                            }

                            strncpy(
                                playlist.titles[playlist.trackCount],
                                title,
                                sizeof(playlist.titles[playlist.trackCount]) - 1
                            );

                            playlist.titles[playlist.trackCount]
                                [sizeof(playlist.titles[playlist.trackCount]) - 1] = '\0';

                            ++playlist.trackCount;
                        } else {
                            if (SoftwareConfig::DEBUG) {
                                Serial.print(F("Could not build track path for: "));
                                Serial.println(fileName);
                            }
                        }
                    } else {
                        if (SoftwareConfig::DEBUG) {
                            Serial.println(
                                F("Playlist full, additional tracks ignored")
                            );
                        }
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
        char title[MAX_TRACK_TITLE_LEN] = {0};

        if (
            parseLine(
                line,
                parsedButton,
                fileName,
                sizeof(fileName),
                title,
                sizeof(title)
            ) &&
            coordinatesEqual(
                parsedButton,
                requestedButton
        )
) {
    if (SoftwareConfig::DEBUG) {
        Serial.print(F("Matched button ["));
        Serial.print(parsedButton.column);
        Serial.print(F("]["));
        Serial.print(parsedButton.row);
        Serial.println(F("]"));
    }    

    if (playlist.trackCount < MAX_PLAYLIST_TRACKS) {
        if (
            buildTrackPath(
                fileName,
                playlist.tracks[playlist.trackCount],
                sizeof(playlist.tracks[playlist.trackCount])
            )
        ) {
            if (SoftwareConfig::DEBUG) {
                Serial.print(F("Track path: "));
                Serial.println(playlist.tracks[playlist.trackCount]);
            }

            ++playlist.trackCount;
        } else {
            if (SoftwareConfig::DEBUG) {
                Serial.print(F("Track path too long: "));
                Serial.println(fileName);
            }
        }
    } else {
        if (SoftwareConfig::DEBUG) {
            Serial.println(
                F("Playlist full, additional tracks ignored")
            );
        }
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
    size_t fileNameSize,
    char* title,
    size_t titleSize
) const
{
    if (
        line == nullptr ||
        fileName == nullptr ||
        fileNameSize == 0 ||
        title == nullptr ||
        titleSize == 0
    ) {
        return false;
    }

    // Minimum format:
    // "00\tT000001.MP3\tTitle"
    if (!isDigit(line[0]) || !isDigit(line[1])) {
        return false;
    }

    if (line[2] != ' ' && line[2] != '\t') {
        return false;
    }

    const ButtonLayout::Coord parsedButton{
        static_cast<uint8_t>(line[0] - '0'),
        static_cast<uint8_t>(line[1] - '0')
    };

    if (!ButtonLayout::isValid(parsedButton)) {
        return false;
    }

    button = parsedButton;

    size_t i = 2;

    // Skip separator after coordinates
    while (line[i] == ' ' || line[i] == '\t') {
        ++i;
    }

    if (line[i] == '\0') {
        return false;
    }

    // --- machine filename ---
    size_t filePos = 0;

    while (
        line[i] != '\0' &&
        line[i] != '\t'
    ) {
        if (filePos < fileNameSize - 1) {
            fileName[filePos++] = line[i];
        }

        ++i;
    }

    fileName[filePos] = '\0';

    if (filePos == 0) {
        return false;
    }

    // No title column:
    // keep old two-column MMS files usable.
    if (line[i] == '\0') {
        strncpy(
            title,
            fileName,
            titleSize - 1
        );

        title[titleSize - 1] = '\0';

        return true;
    }

    // Skip tab between filename and title
    ++i;

    // --- display title ---
    size_t titlePos = 0;

    while (line[i] != '\0') {
        if (titlePos < titleSize - 1) {
            title[titlePos++] = line[i];
        }

        ++i;
    }

    title[titlePos] = '\0';

    // Empty title -> use filename as fallback
    if (titlePos == 0) {
        strncpy(
            title,
            fileName,
            titleSize - 1
        );

        title[titleSize - 1] = '\0';
    }

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
        playlist.titles[i][0] = '\0';
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
        if (SoftwareConfig::DEBUG) {
            Serial.println(F("Failed to open SD root"));
        }
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

        if (SoftwareConfig::DEBUG) {
            Serial.print(F("Checking directory: "));
            Serial.println(directoryName);
        }

        while (true) {
            File entry = directory.openNextFile();

            if (!entry) {
                break;
            }

            if (SoftwareConfig::DEBUG) {
                Serial.print(F("  Entry: "));
                Serial.println(entry.name());
            }

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
                    if (SoftwareConfig::DEBUG) {
                        Serial.println(F("Preset name too long"));
                    }

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
                    if (SoftwareConfig::DEBUG) {
                        Serial.println(F("Preset path too long"));
                    }
                    return false;
                }

                if (
                    registryWritten < 0 ||
                    static_cast<size_t>(registryWritten) >=
                        sizeof(_registryPath)
                ) {
                    if (SoftwareConfig::DEBUG) {
                        Serial.println(F("Registry path too long"));
                    }
                    return false;
                }

                if (SoftwareConfig::DEBUG) {
                    Serial.print(F("Selected preset: "));
                    Serial.println(_presetName);

                    Serial.print(F("Registry path: "));
                    Serial.println(_registryPath);
                }
                
                return true;
            }

            entry.close();
        }

        directory.close();
    }

    root.close();

    if (SoftwareConfig::DEBUG) {
        Serial.println(F("No preset directory with MMS file found"));
    }
    return false;
}