#ifndef GLOBALS_H
#define GLOBALS_H
#pragma warning(push, 0) // disable warnings for libraries
#include "NewEncoder.h"
#include <Arduino.h>
#include <stack>
#include <vector>
#pragma warning(pop) // re-enable warnings

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
#define ROTARY_CLK 34         // pin A/clock of rotary encoder
#define ROTARY_DT 35          // pin B/data of rotary encoder
#define ROTARY_SW 32          // push button of rotary encoder

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

// these global variables are used by the rotary encoder library that will handle our user inputs
extern volatile uint8_t prevEncoderValue; // this variable tracks the most recent input from our rotary encoder and can be used to index the contents of our SD card
extern const uint8_t encoderLowerLimit;   // here we define the lower limit of our encoder value. as a default, this will be zero; once hitting the upper limit, we should cycle around to the lower limit
extern uint8_t encoderUpperLimit;         // this value should be equivalent to the number of files in our directory + 1. this will allow us to iterate over each directory item by turning the rotary encoder.

// our struct here will contain the variables and member functions used to access the contents of our sd card.
struct directoryContents {
  private:
  std::string dirPath = "/"; // this will track our path. working out of root, this structure will only contain '/'; entering a sub-directory will append the name of that subdirectory, and leaving will remove it
  public:
  std::vector<std::string> contents;          // here we will store the contents of the working directory by name, for both printing and accessing
  bool highlighted[DISPLAY_LINES_PER_SCREEN]; // this variable keeps track of which of the displayed lines are highlighted; our "cursor". this allows us to ensure that, on rapid inputs, some lines aren't accidentally left highlighted when they shouldn't be
  uint16_t containedDirs;                     // this variable will keep track of the number of child directories within the working directory. these will always be at the front of our contents vector

  void popWorkingDir(void);                // this function will remove the latest subdirectory from our dirPath as we navigate through our file structure
  void pushWorkingDir(const std::string&); // with this we will add a subdirectory to our dirPath string as we navigate deeper through our file structure
  std::string getDirPath(void);
  uint16_t getDirPathSize(void);
};

extern directoryContents myDir;      // this struct is the container we will use to keep track of the contents of our SD card during operation
extern bool redrawDisplay;           // if the screen needs to be fully redrawn, such as in the case of moving from one page to the next, this variable will be set true and then false again
extern unsigned long timeElapsedOld; // time elapsed since the last time our rotary button was pushed. this will be used to safeguard against unintended button presses or multi-inputs
extern unsigned long timeElapsedNew; // this variable will store the time the moment the rotary button is pushed, which will then be compared against timeElapsedOld

#endif