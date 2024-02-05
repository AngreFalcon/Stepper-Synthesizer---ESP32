#include "sdio.hpp"

// here we will put the definitions for our global SD card library variables
FsFile loadedFile; // this file object is used to open and access the contents of our selected file
SdFs sd;           // this object is used to initialize and access the contents of our SD card

bool initializeSDCard(void) {
  if (SERIAL_DEBUG)
    Serial.print("initializing SD card...");
  if (!sd.begin(SD_CONFIG)) {
    if (SERIAL_DEBUG)
      Serial.println("Failed to initialize SD card.");
    return false;
  }
  if (!sd.exists("/.midi")) {
    sd.mkdir("/.midi");
    makeDirHidden("/.midi");
  }
  if (SERIAL_DEBUG)
    Serial.println("initialization done.");
  readDirectoryContents();
  return true;
}

void readDirectoryContents(void) {
  FsFile myFile, dir;
  std::vector<std::string> tempVec = {};
  char fileName[200];
  myDir.contents.clear();

  while (!dir.open(myDir.getDirPath().c_str()))
    myDir.reinitializeDirPath();
  myFile.openNext(&dir, SD_FILE_READ);

  while (myFile) {
    myFile.getName(fileName, sizeof(fileName));
    if (!myFile.isDirectory() && isMidi(std::string(fileName)))
      tempVec.push_back("/" + std::string(fileName));
    else if (myFile.isDirectory() && !myFile.isHidden())
      myDir.contents.push_back("/" + std::string(fileName));
    myFile.openNext(&dir, SD_FILE_READ);
  }

  std::sort(myDir.contents.begin(), myDir.contents.end());
  std::sort(tempVec.begin(), tempVec.end());
  myDir.containedDirs = myDir.contents.size();
  myDir.contents.insert(myDir.contents.end(), tempVec.begin(), tempVec.end());
  myDir.contents.insert(myDir.contents.begin(), "..");
  encoderUpperLimit = myDir.contents.size();

  if (SERIAL_DEBUG && !updateSettings())
    Serial.println("Couldn't update encoder limits");
  if (SERIAL_DEBUG)
    Serial.println("Done");
  redrawDisplay = true;
  return;
}

void navigateDirectories(void) {
  uint8_t lockedEncoderValue = prevEncoderValue;
  if (lockedEncoderValue >= myDir.contents.size() || (lockedEncoderValue + myDir.getDirPathSize() - 1 == 0)) // ensure that our encoder isn't pointing to a file object that doesn't exist, and that we aren't trying to navigate to a higher directory from root
    return;
  else if ((lockedEncoderValue == 0 || myDir.contents.size() <= 1) && myDir.getDirPathSize() > 1) // if the user wants to navigate to a parent directory, or the current working directory is empty, move up a layer in our file structure
    myDir.popWorkingDir();
  else if (lockedEncoderValue <= myDir.containedDirs && myDir.contents.size() > 1) // if the user selects a file object that is a directory, navigate to that directory
    myDir.pushWorkingDir(myDir.contents[lockedEncoderValue]);
  else { // if all other cases are false, the user selected a file
    openMidi(lockedEncoderValue);
    return;
  }
  readDirectoryContents();
  myDir.highlighted.reset();
  return;
}

bool dirEmpty(FsFile* dir) { // currently not used
  FsFile myFile;
  return !myFile.openNext(dir, SD_FILE_READ);
}

bool isMidi(const std::string& fileName) {
  if (fileName.find(".mid", (std::string(fileName).length() - 4)) == -1)
    return false;
  else
    return true;
}

void makeDirHidden(const std::string& dirName) {
  FsFile myFile;
  myFile.open(dirName.c_str());
  myFile.attrib(FS_ATTRIB_HIDDEN);
  return;
}

bool querySD(void) {
  if (!sd.exists("/.midi")) {
    sd.end();
    return false;
  }
  return true;
}

void openMidi(uint8_t index) {
  FsFile dir;
  dir.open(myDir.getDirPath().c_str());
  loadedFile.open(&dir, myDir.contents[index].c_str(), SD_FILE_READ);
  readMidi();
  return;
}

uint32_t readMidi(void) {
  int16_t readByte = 0;

  if (readHeaderData32() != midiFileContents.headerChunk.headerType || readHeaderData32() != midiFileContents.headerChunk.headerLength) // the purpose of this conditional is to ensure our midi file contains data that is expected. every midi file should start with a consistent header chunk type and length. if our header does not, our midi file is likely invalid
    return -1;
  midiFileContents.headerChunk.headerFormat = readHeaderData16();   // after ensuring our header chunk contains data that is expected, check the format of the midi file
  midiFileContents.headerChunk.headerTrackNum = readHeaderData16(); // once we've checked the midi format, find the number of tracks in the midi file
  midiFileContents.headerChunk.headerDiv = readHeaderData16();      // finally, read in the division information to know what to expect of our midi's delta times
  midiFileContents.allocateTrackChunks();                           // once we know how many track chunks are in our midi, we can allocate the necessary number of struct containers that will store the data for each track
  do {
    readByte = loadedFile.read();

  } while (readByte != -1);
  return 0;
}

uint32_t readHeaderData32() {
  uint32_t headerData = 0;
  int16_t readByte = 0;
  for (uint8_t i = 0; i < sizeof(uint32_t); i++) {
    readByte = loadedFile.read();
    if (readByte == -1)
      break;
    headerData |= ((uint8_t)readByte << (24 - (8 * i))); // shifting bits so that we can read one byte at a time and then store those bytes in our variable
  }
  return headerData;
}

uint16_t readHeaderData16() {
  uint16_t headerData = 0;
  int16_t readByte = 0;
  for (uint8_t i = 0; i < sizeof(uint16_t); i++) {
    readByte = loadedFile.read();
    if (readByte == -1)
      break;
    headerData |= ((uint8_t)readByte << (8 - (8 * i))); // shifting bits so that we can read one byte at a time and then store those bytes in our variable
  }
  return headerData;
}