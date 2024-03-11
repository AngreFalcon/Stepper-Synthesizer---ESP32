#ifndef GLOBALS_H
#define GLOBALS_H
#include <Arduino.h>
#include <atomic>

// this macro is intended to enable or disable serial monitor print statements
#define SERIAL_DEBUG true

// this value will be used to connect to our device via serial monitor
#define SERIAL_BAUD_RATE 115200

// the following macros will be used to define what pins
// are being used by our ESP32
// note: SPI pins names can vary; aliases listed below
// DIN typically refers to the input pin of the master device
// whereas DOUT represents the input pin of the slave device
// MISO = SOMI; SDI; DI; SI; MRST
// MOSI = SIMO; SDO; DO; SO; MTSR
// SCLK = SCK; CLK
// SS = nCS; CS; CSB; CSN; nSS; STE; SYNC

// SDA pin for SD card indicator
#define SD_INDIC_SDA 26

// SCL pin for SD card indicator
#define SD_INDIC_SCL 27

// I2C address for SD card indicator
#define SD_INDIC_ADDRESS 0x3C

// horizontal resolution for SD card indicator
#define SD_INDIC_WIDTH 128

// vertical resolution for SD card indicator
#define SD_INDIC_HEIGHT 64

// reset pin for SD card indicator
// -1 indicates reset is shared with microcontroller reset
#define SD_INDIC_RESET -1

// pin A/clock of rotary encoder
#define ROTARY_CLK 36

// pin B/data of rotary encoder
#define ROTARY_DT 39

// push button of rotary encoder
#define ROTARY_SW 34

// pins for stepper motor actuation
#define MTR_STEPS 4, 32, 33, 25

/*VSPI*/
// master input/slave output for SD card reader
#define SD_MISO 19

// master output/slave input for SD card reader
#define SD_MOSI 23

// CLK for SD card reader
#define SD_SCLK 18

// chip select for SD card reader
#define SD_CS 5
/*VSPI*/

/*HSPI*/
// master input/slave output for LCD. used for data command (DC) pin
#define LCD_DC 12

// master output/slave input for LCD. used for data input (DIN) pin
#define LCD_MOSI 13

// CLK for LCD
#define LCD_SCLK 14

// chip select for LCD
#define LCD_CS 15

// reset for LCD
#define LCD_RST 16

// backlight for LCD. set low to disable, high to enable
// currently t to -1 to reduce pin usage; wired directly to VCC
#define LCD_BL -1
/*HSPI*/

// here we will define the macros that we intend to use for our display
// these macros are for convenience; these values may already
// be defined elsewhere, but implementing our own macros here will
// allow us to modify by only changing one value instead of changing
// multiple function calls throughout our program
// this will also allow us to define our own custom colors,
// font attributes, and other values if we so choose

// background color for our display
#define DISPLAY_BG 0x4208

// color for our text
#define DISPLAY_TEXT 0xbe5b

// highlight color for our selected menu option
#define DISPLAY_HL 0xf40e

// color for our highlighted text
#define DISPLAY_TEXT_HL 0xd5fa

// the font we will use for our menu
#define DISPLAY_FONT 4

// the number of vertical pixels needed for each char of text
#define DISPLAY_FONT_HEIGHT 26

// the number of pixels used to pad our text to keep each line from touching
#define DISPLAY_FONT_VERTICAL_PADDING 4

// the number of pixels used to keep our text from touching the left border of the
#define DISPLAY_FONT_HORIZONTAL_PADDING 5

// this number indicates the number of lines we can render on screen at once
#define DISPLAY_LINES_PER_SCREEN 8

// this value is equivalent to the horizontal resolution of our display
// and is used to fully clear individual lines of text
#define DISPLAY_HORIZONTAL_PADDING 320

// these global variables are used by the rotary encoder library that will handle our user inputs

// this variable tracks the most recent input from our rotary encoder
// and can be used to index the contents of our SD card
extern volatile uint8_t prevEncoderValue;

// this value should be equivalent to the number of files in our directory + 1
// this will allow us to iterate over each directory item by turning the rotary encoder
extern uint8_t encoderUpperLimit;

// if the screen needs to be fully redrawn, such as in the case of moving
// from one page to the next, this variable will be set true
// and then false again
extern bool redrawDisplay;

// time elapsed since the last time our rotary button was pushed
// this will be used to safeguard against unintended button presses
// or multi-inputs
extern volatile std::atomic<unsigned long> timeElapsedOld;

// this variable will store the time the moment the rotary button is pushed
// which will then be compared against timeElapsedOld
// extern volatile unsigned long timeElapsedNew;

extern volatile std::atomic<bool> rotaryButtonPressed;

#endif