#ifndef SDIO_H
#define SDIO_H
#include "SdFat.h"

// these macros are required for initialization of our SD card reader
// first we specify the clock speed our our SPI bus
// then we define our SD card's CS pin number and whether our SPI bus is being used by other devices or not
#define SPI_CLOCK SD_SCK_MHZ(16)
#define SD_CONFIG SdSpiConfig(SD_CS, SHARED_SPI, SPI_CLOCK)

// these macros are initially defined in SD.h and are used by the SD card library, but are redefined in FS.h
// therefore they must be redefined here to avoid compile errors
#define SD_FILE_READ O_RDONLY
#define SD_FILE_WRITE (O_RDWR | O_CREAT | O_AT_END)

bool initializeSDCard(void);                    // initialize our SdFs object and perform any other operations necessary for the use of our SD card
void readDirectoryContents(void);               // read the contents of our current working directory
void navigateDirectories(void);                 // based on the current value of our rotary encoder, enter or exit a subdirectory or open a file from the current working directory
bool dirEmpty(FsFile* dir);                     // return true if the specified directory is empty. else, return false. wrapper function for FsFile::openNext
bool isMidi(const std::string& fileName);       // return true if the specified file object is a midi file. else, return false
void makeDirHidden(const std::string& dirName); // makes a directory hidden. useful for preventing directories from being displayed on our display, as hidden files and folders are ignored
bool querySD(void);                             // used to check if our SD card is still detected and read/writable
void openMidi(const uint8_t index);             // opens midi file and makes various function calls to parse the data within

#endif