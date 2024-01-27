#ifndef SDIO_H
#define SDIO_H
#pragma warning(push, 0) // disable warnings for libraries
#include "SdFat.h"
#include "globals.hpp"
#include "rotary.hpp"
#pragma warning(pop) // re-enable warnings

// these macros are required for initialization of our SD card reader
// first we specify the clock speed our our SPI bus
// then we define our SD card's CS pin number and whether our SPI bus is being used by other devices or not
#define SPI_CLOCK SD_SCK_MHZ(16)
#define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SPI_CLOCK)

// these macros are initially defined in SD.h and are used by the SD card library, but are redefined in FS.h
// therefore they must be redefined here to avoid compile errors
#define SD_FILE_READ O_RDONLY
#define SD_FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)

// here we will declare any global variables used by our SD card library
extern SdFs sd;
extern FsFile loadedFile;

bool initializeSDCard();         // initialize our SdFs object and perform any other operations necessary for the use of our SD card
void readDirectoryContents();    // read the contents of our current working directory
void navigateDirectories();      // based on the current value of our rotary encoder, enter or exit a subdirectory or open a file from the current working directory
bool dirEmpty(FsFile*);          // return true if the specified directory is empty. else, return false. wrapper function for FsFile::openNext
bool isMidi(const std::string&); // return true if the specified file object is a midi file. else, return false

#endif