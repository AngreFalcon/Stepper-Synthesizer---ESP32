#include "sdio.hpp"
#include "globals.hpp"
#include "midi.hpp"
#include "rotary.hpp"
#include "sdio-directoryContents.hpp"
#include <queue>

// here we will put the definitions for our global SD card library variables
FsFile loadedFile; // this file object is used to open and access the contents of our selected file
SdFs sd;           // this object is used to initialize and access the contents of our SD card

bool initializeSDCard(void) {
  if (SERIAL_DEBUG) {
    Serial.print("initializing SD card...");
  }
  if (!sd.begin(SD_CONFIG)) {
    if (SERIAL_DEBUG) {
      Serial.println("Failed to initialize SD card.");
    }
    return false;
  }
  if (!sd.exists("/.midi")) {
    sd.mkdir("/.midi");
    makeDirHidden("/.midi");
  }
  if (SERIAL_DEBUG) {
    Serial.println("initialization done.");
  }
  readDirectoryContents();
  return true;
}

void readDirectoryContents(void) {
  FsFile myFile, dir;
  std::vector<std::string> tempVec = {};
  char fileName[200];
  myDir.contents.clear();

  while (!dir.open(myDir.getDirPath().c_str())) {
    myDir.reinitializeDirPath();
  }
  myFile.openNext(&dir, SD_FILE_READ);

  while (myFile) {
    myFile.getName(fileName, sizeof(fileName));
    if (!myFile.isDirectory() && isMidi(std::string(fileName))) {
      tempVec.push_back("/" + std::string(fileName));
    }
    else if (myFile.isDirectory() && !myFile.isHidden()) {
      myDir.contents.push_back("/" + std::string(fileName));
    }
    myFile.openNext(&dir, SD_FILE_READ);
  }

  std::sort(myDir.contents.begin(), myDir.contents.end());
  std::sort(tempVec.begin(), tempVec.end());
  myDir.containedDirs = myDir.contents.size();
  myDir.contents.insert(myDir.contents.end(), tempVec.begin(), tempVec.end());
  myDir.contents.insert(myDir.contents.begin(), "..");
  encoderUpperLimit = myDir.contents.size();

  if (SERIAL_DEBUG && !updateSettings()) {
    Serial.println("Couldn't update encoder limits");
  }
  if (SERIAL_DEBUG) {
    Serial.println("Done");
  }
  redrawDisplay = true;
  return;
}

void navigateDirectories(void) {
  uint8_t lockedEncoderValue = prevEncoderValue;
  if (lockedEncoderValue >= myDir.contents.size() || (lockedEncoderValue + myDir.getDirPathSize() - 1 == 0)) { // ensure that our encoder isn't pointing to a file object that doesn't exist, and that we aren't trying to navigate to a higher directory from root
    return;
  }
  else if ((lockedEncoderValue == 0 || myDir.contents.size() <= 1) && myDir.getDirPathSize() > 1) { // if the user wants to navigate to a parent directory, or the current working directory is empty, move up a layer in our file structure
    myDir.popWorkingDir();
  }
  else if (lockedEncoderValue <= myDir.containedDirs && myDir.contents.size() > 1) { // if the user selects a file object that is a directory, navigate to that directory
    myDir.pushWorkingDir(myDir.contents[lockedEncoderValue]);
  }
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
  if (fileName.find(".mid", (std::string(fileName).length() - 4)) == -1) {
    return false;
  }
  else {
    return true;
  }
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
  else {
    return true;
  }
}

void openMidi(const uint8_t index) {
  FsFile dir;
  dir.open(myDir.getDirPath().c_str());
  loadedFile.open(&dir, myDir.contents[index].c_str(), SD_FILE_READ);
  while (loadedFile.peek() != -1) {
    songData.writeByte((uint8_t)loadedFile.read());
  }
  loadedFile.close();
  songData.parseMidi();
  songData.playMidi();
  return;
}