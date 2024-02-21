#include "sdio-directoryContents.hpp"

// this struct is the container we will use to keep track
// of the contents of our SD card during operation
directoryContents myDir; 

void directoryContents::popWorkingDir(void) {
  // remove characters from the end of our path name
  // until we find the first instance of a forward slash
  while (dirPath.back() != '/') { 
    dirPath.pop_back();
  }

  // because we stopped removing characters from the end of our path name
  // upon finding the first instance of a forward slash, we still need
  // to pop from the back of our string to remove that forward slash
  dirPath.pop_back(); 
  return;
}

void directoryContents::pushWorkingDir(const std::string& parent) {
  dirPath.append(parent);
  return;
}

std::string directoryContents::getDirPath(void) {
  return dirPath;
}

uint16_t directoryContents::getDirPathSize(void) {
  return dirPath.size();
}

void directoryContents::reinitializeDirPath(void) {
  dirPath = "/";
  return;
}