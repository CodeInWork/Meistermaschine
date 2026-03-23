#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

constexpr uint8_t CARDCS = 4;
constexpr uint8_t VS1053_CS = 10;
constexpr uint8_t VS1053_DCS = 8;
constexpr uint8_t VS1053_RESET = 9;

void setup()
{
    Serial.begin(9600);
    delay(1000);

    pinMode(VS1053_CS, OUTPUT);
    pinMode(VS1053_DCS, OUTPUT);
    pinMode(VS1053_RESET, OUTPUT);
    pinMode(CARDCS, OUTPUT);

    // VS1053 sicher stilllegen
    digitalWrite(VS1053_CS, HIGH);
    digitalWrite(VS1053_DCS, HIGH);
    digitalWrite(VS1053_RESET, LOW);

    // SD deselektiert, dann später selektiert von SD.begin()
    digitalWrite(CARDCS, HIGH);

    Serial.println(F("Starting SD-only test"));

    if (!SD.begin(CARDCS)) {
        Serial.println(F("SD.begin failed"));
        return;
    }

    Serial.println(F("SD.begin OK"));

    Serial.print(F("exists(reg.mms) = "));
    Serial.println(SD.exists("reg.mms"));

    File root = SD.open("/");
    Serial.print(F("root valid = "));
    Serial.println(static_cast<bool>(root));

    if (root) {
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
    }

    File f = SD.open("reg.mms", FILE_READ);
    Serial.print(F("open(reg.mms) = "));
    Serial.println(static_cast<bool>(f));

    if (f) {
        Serial.print(F("size = "));
        Serial.println(f.size());

        Serial.println(F("Content:"));
        while (f.available()) {
            Serial.write(f.read());
        }
        Serial.println();
        f.close();
    }
}

void loop()
{
}