#include "sdio.hpp"

// here we will put the definitions for our global SD card library variables
FsFile loadedFile;
SdFs sd;

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

  dir.open(myDir.getDirPath().c_str());
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
    Serial.println("we selected a file");
    return;
  }
  myDir.contents.clear();
  readDirectoryContents();
  memset(myDir.highlighted, false, sizeof(myDir.highlighted));
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
	if (!sd.exists("/.midi"))
		return false;
	return true;
}