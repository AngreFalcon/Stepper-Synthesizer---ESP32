#ifndef SDIO_DIRECTORYCONTENTS_HPP
#define SDIO_DIRECTORYCONTENTS_HPP
#include <bitset>
#include <vector>

// our struct here will contain the variables
// and member functions used to access the contents of our sd card
class directoryContents {
  // this will track our path. working out of root, this structure
  // will only contain '/'; entering a sub-directory will append
  // the name of that subdirectory, and leaving will remove it
  std::string dirPath = "/"; 

  public:
  // here we will store the contents of the working directory by name
  // for both printing and accessing
  std::vector<std::string> contents; 

  // this variable keeps track of which of the displayed lines
  // are highlighted - our "cursor". this allows us to ensure that
  // on rapid inputs, some lines aren't accidentally left highlighted
  // when they shouldn't be
  std::bitset<8> highlighted;      

  // this variable will keep track of the number of child directories
  // within the working directory. these will always be
  // at the front of our contents vector  
  uint16_t containedDirs;            

  // this function will remove the latest subdirectory
  // from our dirPath as we navigate through our file structure
  void popWorkingDir(void);                

  // with this we will add a subdirectory to our dirPath string
  // as we navigate deeper through our file structure
  void pushWorkingDir(const std::string& parent); 

  // this function will return our working directory path
  std::string getDirPath(void);            

  // here we can get the current length of our working directory path
  uint16_t getDirPathSize(void);         

  // should we need to reinitialize our directory path to root
  // we can do so by calling this function  
  void reinitializeDirPath(void);          
};

// this struct is the container we will use to keep track
// of the contents of our SD card during operation
extern directoryContents myDir; 

#endif