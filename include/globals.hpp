#ifndef GLOBALS_H
#define GLOBALS_H
#include "NewEncoder.h"
#include <Arduino.h>
#include <array>
#include <bitset>
#include <stack>
#include <vector>
#include <limits>

#define SERIAL_DEBUG true       // this macro is intended to enable or disable serial monitor print statements
#define SERIAL_BAUD_RATE 115200 // this value will be used to connect to our device via serial monitor

// the following macros will be used to define what pins are being used by our ESP32
// note: SPI pins names can vary; aliases listed below
// DIN typically refers to the input pin of the master device, whereas DOUT represents the input pin of the slave device
// MISO = SOMI; SDI; DI; SI; MRST
// MOSI = SIMO; SDO; DO; SO; MTSR
// SCLK = SCK; CLK
// SS = nCS; CS; CSB; CSN; nSS; STE; SYNC
#define SD_INDIC_SDA 26       // SDA pin for SD card indicator
#define SD_INDIC_SCL 27       // SCL pin for SD card indicator
#define SD_INDIC_ADDRESS 0x3C // I2C address for SD card indicator
#define SD_INDIC_WIDTH 128    // horizontal resolution for SD card indicator
#define SD_INDIC_HEIGHT 64    // vertical resolution for SD card indicator
#define SD_INDIC_RESET -1     // reset pin for SD card indicator. -1 indicates reset is shared with microcontroller reset
#define ROTARY_CLK 36         // pin A/clock of rotary encoder
#define ROTARY_DT 39          // pin B/data of rotary encoder
#define ROTARY_SW 34          // push button of rotary encoder
#define MTR_STEP 4            // pin for stepper motor actuation

/*VSPI*/
#define SD_MISO 19 // master input/slave output for SD card reader
#define SD_MOSI 23 // master output/slave input for SD card reader
#define SD_SCLK 18 // CLK for SD card reader
#define SD_CS 5    // chip select for SD card reader
/*VSPI*/

/*HSPI*/
#define LCD_DC 12   // master input/slave output for LCD. used for data command (DC) pin
#define LCD_MOSI 13 // master output/slave input for LCD. used for data input (DIN) pin
#define LCD_SCLK 14 // CLK for LCD
#define LCD_CS 15   // chip select for LCD
#define LCD_RST 16  // reset for LCD
#define LCD_BL 17   // backlight for LCD. set low to disable, high to enable
/*HSPI*/

// here we will define the macros that we intend to use for our display
// these macros are for convenience; these values may already be defined elsewhere, but implementing our own macros here will
// allow us to modify by only changing one value instead of changing multiple function calls throughout our program
// this will also allow us to define our own custom colors, font attributes, and other values if we so choose
#define DISPLAY_BG 0x4208                 // background color for our display
#define DISPLAY_TEXT 0xbe5b               // color for our text
#define DISPLAY_HL 0xf40e                 // highlight color for our selected menu option
#define DISPLAY_TEXT_HL 0xd5fa            // color for our highlighted text
#define DISPLAY_FONT 4                    // the font we will use for our menu
#define DISPLAY_FONT_HEIGHT 26            // the number of vertical pixels needed for each char of text
#define DISPLAY_FONT_VERTICAL_PADDING 4   // the number of pixels used to pad our text to keep each line from touching
#define DISPLAY_FONT_HORIZONTAL_PADDING 5 // the number of pixels used to keep our text from touching the left border of the
#define DISPLAY_LINES_PER_SCREEN 8        // this number indicates the number of lines we can render on screen at once
#define DISPLAY_HORIZONTAL_PADDING 320    // this value is equivalent to the horizontal resolution of our display and is used to fully clear individual lines of text

#define CHUNKTYPE_HEADER 0x4d546864 // this macro is used to keep track of the midi chunk type used by headers, and is the hexadecimal representation of MThd
#define CHUNKTYPE_TRACK 0x4d54726b  // this macro is used to keep track of the midi chunk type used by tracks, and is the hexadecimal representation of MTrk

// these global variables are used by the rotary encoder library that will handle our user inputs
extern volatile uint8_t prevEncoderValue; // this variable tracks the most recent input from our rotary encoder and can be used to index the contents of our SD card
extern const uint8_t encoderLowerLimit;   // here we define the lower limit of our encoder value. as a default, this will be zero; once hitting the upper limit, we should cycle around to the lower limit
extern uint8_t encoderUpperLimit;         // this value should be equivalent to the number of files in our directory + 1. this will allow us to iterate over each directory item by turning the rotary encoder.

// our struct here will contain the variables and member functions used to access the contents of our sd card.
class directoryContents {
  std::string dirPath = "/"; // this will track our path. working out of root, this structure will only contain '/'; entering a sub-directory will append the name of that subdirectory, and leaving will remove it

  public:
  std::vector<std::string> contents; // here we will store the contents of the working directory by name, for both printing and accessing
  std::bitset<8> highlighted;        // this variable keeps track of which of the displayed lines are highlighted; our "cursor". this allows us to ensure that, on rapid inputs, some lines aren't accidentally left highlighted when they shouldn't be
  uint16_t containedDirs;            // this variable will keep track of the number of child directories within the working directory. these will always be at the front of our contents vector

  void popWorkingDir(void);                // this function will remove the latest subdirectory from our dirPath as we navigate through our file structure
  void pushWorkingDir(const std::string&); // with this we will add a subdirectory to our dirPath string as we navigate deeper through our file structure
  std::string getDirPath(void);            // this function will return our working directory path
  uint16_t getDirPathSize(void);           // here we can get the current length of our working directory path
  void reinitializeDirPath(void);          // should we need to reinitialize our directory path to root, we can do so by calling this function
};

// this struct will be used to store the contents of our MIDI file once loaded. much of the information this is based off of is thanks to the documentation at https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
// credit goes to David Back of mcgill.ca for the descriptions of some of the member variables below
struct midiContents {
  struct {
    const uint32_t headerType = CHUNKTYPE_HEADER; // this value represents the type of chunk. for a header chunk, this value should always equate to the ASCII value of "MThd"
    const uint32_t headerLength = 6;              // this value indicates how long in bytes the body of the header chunk is. this value should always evaluate to 6
    uint16_t headerFormat;                        // if this evaluates to 0, MIDI contains a single multi-channel track. 1 indicates one or more simultaneous ttracks or MIDI outputs of a sequence. 2 indicates one or more sequentially independent single-track patterns
    uint16_t headerTrackNum;                      // number of tracks in the MIDI file. will always be 1 for a format 0 file
    uint16_t headerDiv;                           // specifies meaning of delta-times. has two formats, one for metrical time and one for time-code-based time
  } headerChunk;
  struct trackChunk {
    const uint32_t trackType = CHUNKTYPE_TRACK; // this value represents the type of chunk. for a track chunk, this value should always equate to the ASCII value of "MTRK"
    uint32_t trackLength = 0;                   // this value represents in bytes how long the body of data for this chunk is
    std::vector<uint8_t> trackData;             // this vector contains the data from our track chunk, stored in bytes
  };

  std::vector<trackChunk> trackChunkArray; // this vector will keep track of the various track chunks and their data in our midi file

  void allocateTrackChunks(); // this function is called while reading our midi file and is used to allocate an appropriate number of elements in our trackChunkArray corresponding to the number of track chunks in our midi file
};

extern midiContents midiFileContents; // our object here will be used to store the contents of our midi file once it is selected by the user
extern directoryContents myDir;       // this struct is the container we will use to keep track of the contents of our SD card during operation
extern bool redrawDisplay;            // if the screen needs to be fully redrawn, such as in the case of moving from one page to the next, this variable will be set true and then false again
extern unsigned long timeElapsedOld;  // time elapsed since the last time our rotary button was pushed. this will be used to safeguard against unintended button presses or multi-inputs
extern unsigned long timeElapsedNew;  // this variable will store the time the moment the rotary button is pushed, which will then be compared against timeElapsedOld

#endif