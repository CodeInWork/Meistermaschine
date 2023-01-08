#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


// define the pins used
#define CLK 13       // SPI Clock, shared with SD card
#define MISO 12      // Input data, from VS1053/SD card
#define MOSI 11      // Output data, to VS1053/SD card
#define BREAKOUT_RESET  9      // VS1053 reset pin (output)
#define BREAKOUT_CS     10     // VS1053 chip select pin (output)
#define BREAKOUT_DCS    8      // VS1053 Data/command select pin (output)
#define CARDCS 4     // Card chip select pin
#define DREQ 3       // VS1053 Data request, ideally an Interrupt pin


#define VolumePin A0
#define changeFilePin 2   //interrupt pin NOT USED

#define buttonPin1 5
#define buttonPin2 6

//define variables
File reg;
const uint8_t fileNameSize = 42;
const uint8_t fileIDSize = 5;
const uint8_t fileFormatSize = 5;
const uint8_t regSize = 10;

const char *registryPath = "/mp3/reg.txt";
const char *filePath = "/mp3";

//char registry[regSize][fileNameSize];

struct registryEntry {
  char *fileID;
  char *fileName;
  char *fileFormat;
};

struct registryEntry curEntry;



//create player object
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);


// Set the LCD address to 0x20 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);


void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. 
  }

  Serial.println(F("AmbGen Test"));
  /**
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053"));
     while (1);
  }
  **/
  if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }


  // Initialize lcd display (I2C bus)
  lcd.init(); 
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("AmbGen ready"); 

  curEntry = readRegistryEntry(registryPath, "2");
  
  displayTitleLCD(curEntry.fileName, 1);
  displayTitleLCD("", 0);

}

void loop() {
  // put your main code here, to run repeatedly:

}


/*************************************************** 
Display title on lcd
 ****************************************************/
void displayTitleLCD(char *title, uint8_t line) {
  if (line < 2) {
    for (uint8_t i=0; i<16; i++){  //clear line
      lcd.setCursor(i,line);
      lcd.write(32);
    }

    uint8_t len = strlen(title);
    for (uint8_t i=0; i<len; i++){  //write title
      lcd.setCursor(i,line);
      lcd.write(*(title+i));
    }
  } else {
    Serial.println(F("line number exceeds display size!"));
  }
}


/*************************************************** 
List Files on SD card
 ****************************************************/
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       dir.rewindDirectory();
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}


struct registryEntry readRegistryEntry(char *registryPath, char *buttonID){
  //Serial.println("reg.txt:");
    
  if (!SD.exists(registryPath)) {
    Serial.println("reg.txt doesn't exist.");
    while(1);
  } 

  File reg = SD.open(registryPath);

  if (reg) {
    struct registryEntry regEntry;
    char *delim[] = {"\t", "."};
    //char delim1[2] = "\t";
    //char delim2[2] = "."; 
    char line[fileNameSize+fileIDSize+fileFormatSize+1];
    while (reg.available()) {

      uint8_t lineLength = reg.readBytesUntil('\n', line, fileNameSize+fileIDSize+fileFormatSize+1);
      line[lineLength] = '\0';    //terminate char array
      
      if(lineLength >= fileNameSize+fileIDSize+fileFormatSize+1){
        Serial.println("File name exceeds limit.");
        //read rest of line in File;
        while (1) {
          char c = reg.read();
          if (c == '\n') {
            break; 
          }
        }

      } else {
        //Serial.println(line);
        char *fileID; 
        fileID = strtok(line, *delim);
        
        if(strcmp(fileID, buttonID) == 0){
          regEntry.fileID = fileID;
          //Serial.println(regEntry.fileID);
          regEntry.fileName = strtok(NULL, *(delim+1));
          //Serial.println(regEntry.fileName);
          regEntry.fileFormat = strtok(NULL, *(delim+1));
          reg.close();
          return regEntry;
          //Serial.println(curEntry.fileFormat);
        }
      }
    } 
  reg.close();
  } else {
    Serial.println("Error opening File!");
  }
}

