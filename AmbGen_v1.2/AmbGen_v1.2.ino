/*************************************************** 



 ****************************************************/

// include SPI, MP3 and SD libraries
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
File root;

const uint8_t fileNameSize = 22;

char soundFile[fileNameSize];

volatile bool playingSound = false;

volatile uint8_t fileNumber = 0;
volatile uint8_t buttonNum = 0;
volatile uint8_t volume = 20;     // Default volume; 0 max, 100 min volume



//create player object
Adafruit_VS1053_FilePlayer musicPlayer = 
  Adafruit_VS1053_FilePlayer(BREAKOUT_RESET, BREAKOUT_CS, BREAKOUT_DCS, DREQ, CARDCS);


// Set the LCD address to 0x20 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

/*************************************************** 
Function to change sound file if button is pushed
 ****************************************************/
void changeSoundFilePlayed(){
  
  if(digitalRead(buttonPin1)){
    buttonNum = 1;
  }
  else if(digitalRead(buttonPin2)) {
    buttonNum = 2;    
  }

  if(buttonNum != fileNumber){
    fileNumber = buttonNum;
    uint8_t oldVolume = volume;
    
    if (!musicPlayer.stopped()) {
      crossFade(100, 40, 400);             //fade out old to minimum volume
      playingSound = playSoundFile(fileNumber);
      crossFade(oldVolume, 40, 400);        //fade in new to old volume
    } else {
      playingSound = playSoundFile(fileNumber);
    }
    
    //update lcd 
    lcd.clear();
    lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile. 
    lcd.print(soundFile); 
  }
}


/*************************************************** 
Plays Sound File from SD card specified by an integer number
 ****************************************************/
bool playSoundFile(uint8_t number){
  //root.rewindDirectory();
  //findFile(root, number);
  setFileName(root, 0 , number);
  musicPlayer.stopPlaying();
  Serial.println("flag 1");
  return  musicPlayer.startPlayingFile(soundFile);
}

/*************************************************** 
Fades volume of played sound to targetVolume
 ****************************************************/
void crossFade(uint8_t targetVolume, uint8_t steps, uint16_t time_ms) {
  int8_t stepWidth = (targetVolume-volume) / steps;
  int8_t delayTime = time_ms/steps;
  for (uint8_t i = 1; i <= steps; i++) {
    volume = volume + stepWidth;
    musicPlayer.setVolume(volume, volume);
    delay(delayTime);
  }
  musicPlayer.setVolume(targetVolume, targetVolume);
  volume = targetVolume;
}


void displayTitleLCD() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(soundFile); 

}


/*************************************************** 
Gets Volume by reading voltage at VolumePin and sets it
 ****************************************************/
void getSetVolume() {
  volume = map(analogRead(VolumePin), 0, 1023, 100, 0);
  musicPlayer.setVolume(volume,volume);
  //Serial.println(volume);
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

/*************************************************** 
Find File with number and declares public variable <soundFile>
 ****************************************************/
void findFile(File dir, uint8_t fileNumber) {
  uint8_t digitCount = countDigit(fileNumber);
  
  char numberID[digitCount+2];
  char fileID[digitCount+2];

  Serial.print("digit count is: ");
  Serial.println(digitCount);

  sprintf(numberID,"%d_", fileNumber);
  Serial.print("Number ID is: ");
  Serial.println(numberID);
  
   while(true) {
     
     File entry =  dir.openNextFile();
     
     if (! entry) {
       // no more files
       Serial.println("No more files");
       dir.rewindDirectory();
       break;
     }

     Serial.print("entry name is: ");
     Serial.println(entry.name());

     if (!entry.isDirectory()) {
       strncpy(fileID, entry.name(), digitCount+1);
       fileID[digitCount+1] = '\0'; 
       Serial.print("File ID is: ");
       Serial.println(fileID);

       if (strcmp(fileID,numberID) == 0) {
        strncpy(soundFile, entry.name(), fileNameSize); 
        Serial.println(soundFile);        
       }  
     } 

     entry.close();
     
   }
}

void setFileName(File dir, int numTabs, uint8_t fileNumber) {
  uint8_t digitCount = countDigit(fileNumber);
  
  char numberID[digitCount+2];
  char fileID[digitCount+2];

  Serial.print("digit count is: ");
  Serial.println(digitCount);

  sprintf(numberID,"%d_", fileNumber);
  Serial.print("Number ID is: ");
  Serial.println(numberID);
   
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       dir.rewindDirectory();
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       setFileName(entry, numTabs+1, fileNumber);
     } else {
       strncpy(fileID, entry.name(), digitCount+1);
       fileID[digitCount+1] = '\0'; 
       Serial.print("File ID is: ");
       Serial.println(fileID);

       if (strcmp(fileID,numberID) == 0) {
        strncpy(soundFile, entry.name(), fileNameSize); 
        Serial.println(soundFile);        
       }  
     }
     entry.close();
   }
}


/*************************************************** 
Count digits of a integer number (for number of track to play)
 ****************************************************/
uint8_t countDigit(uint8_t number)
{
  if (number/10 == 0) {
    return 1;
  } else {
    return 1 + countDigit(number / 10);
  }
}

  
void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. 
  }

  Serial.println(F("AmbGen Test"));

  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053"));
     while (1);
  }
  Serial.println(F("VS1053 found"));
  
   if (!SD.begin(CARDCS)) {
    Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }

  // list files
  root = SD.open("/");
  //printDirectory(root, 0);
  
  if(SD.exists("1_~")){
       Serial.println("it exists!");
  }

  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
  
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  // Initialize lcd display (I2C bus)
  lcd.init(); 
  lcd.backlight();

  lcd.setCursor(0, 0);//Hier wird die Position des ersten Zeichens festgelegt. In diesem Fall bedeutet (0,0) das erste Zeichen in der ersten Zeile. 
  lcd.print("AmbGen ready"); 

  //attach interrupt to pin for changing file to play
  //attachInterrupt(digitalPinToInterrupt(changeFilePin), changeSoundFilePlayed, RISING);

  // musicPlayer.sineTest(0x44, 500); // Make a tone to indicate VS1053 is working

}

void loop() {
  getSetVolume();
  changeSoundFilePlayed();
  delay(100);
  // File is playing in the background
  if(playingSound && musicPlayer.stopped()){
    //playSoundFile(fileNumber);
  }
  
}




