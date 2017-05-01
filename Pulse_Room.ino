/*  Pulse Room for Feather M0   by Collin Cunningham for Adafruit Industries
    based on Pulse Sensor Amped 1.4    by Joel Murphy and Yury Gitman

  - Uses a Feather M0, Feather MusicMaker FeatherWing, & Pulse Sensor Amped
  - Pulse Sensor Amped connected to pin A0
  - PWM signal outputs on pin 11

  note: this code working with Arduino IDE v1.8.2, Adafruit SAMD library installed & Arduino SAMD Boards library *not* installed

  more @ https://learn.adafruit.com
*/

//audio playback
#include <SPI.h>
#include <Adafruit_VS1053.h>
#include <SD.h>
#include <Adafruit_ZeroTimer.h>

#define VS1053_RESET   -1     // VS1053 reset pin (not used!)
#define VS1053_CS       6     // VS1053 chip select pin (output)
#define VS1053_DCS     10     // VS1053 Data/command select pin (output)
#define CARDCS          5     // Card chip select pin
// DREQ should be an Int pin *if possible* (not possible on 32u4)
#define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin
#define FILE_NAME "beat1.mp3" //name of audio file to be played at each beat

Adafruit_ZeroTimer zt4 = Adafruit_ZeroTimer(4);

boolean mute = true;
int mutePin = 12;

//  Pulse Variables
int pulsePin = 14;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 11;                   // pin to do fancy classy fading blink at each beat
int fadeRate = 0;                 // used to fade LED on with PWM on fadePin

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

Adafruit_VS1053_FilePlayer audio = Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);

void setup() {
  pinMode(blinkPin, OUTPUT);        // pin that will blink to your heartbeat
  pinMode(fadePin, OUTPUT);         // pin that will fade to your heartbeat
  pinMode(mutePin, INPUT_PULLUP);          // pin that mutes sound and PWM

  audioSetup();                     // set up vs1053 first
  pulseSetup();                     // set up pulse sensor second

  pinMode(1, INPUT_PULLUP);         //fix for VS1053

  audio.startPlayingFile(FILE_NAME);  //test audio playback
  delay(1000);
}


void loop() {

  if (digitalRead(mutePin) == LOW) {
    mute = false;
  }
  else {
    analogWrite(fadePin, 0);
    mute = true;
  }

  if (QS == true) {    // A Heartbeat Was Found
    // BPM and IBI have been Determined
    // Quantified Self "QS" true when arduino finds a heartbeat
    fadeRate = 255;         // Makes the LED Fade Effect Happen
    QS = false;                      // reset the Quantified Self flag for next time

    if (audio.playingMusic) {
      audio.stopPlaying();
    }
    if (!mute) {
      audio.startPlayingFile(FILE_NAME);
    }
  }
  if (!mute) {
    ledFadeToBeat();                      // Makes the LED Fade Effect Happen
  }
  delay(20);                             //  take a break
}


void ledFadeToBeat() {
  fadeRate -= 15;                         //  set LED fade value
  fadeRate = constrain(fadeRate, 0, 255); //  keep LED fade value from going into negative numbers!
  analogWrite(fadePin, fadeRate);         //  fade LED
}


void audioSetup() {
  audio.begin(); // initialise the music player
  audio.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working
  delay(1000);
  SD.begin(CARDCS);    // initialise the SD card
  // Set volume for left, right channels. lower numbers == louder volume!
  audio.setVolume(0, 0);
  audio.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int on M0
}

