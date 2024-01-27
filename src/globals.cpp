#include "globals.hpp"

volatile uint8_t prevEncoderValue;
const uint8_t encoderLowerLimit = 0;
uint8_t encoderUpperLimit = 20;

directoryContents myDir;

bool redrawDisplay = false;
unsigned long timeElapsedOld = 0;
unsigned long timeElapsedNew = 0;

void directoryContents::popWorkingDir() {
	while (dirPath.back() != '/')
		dirPath.pop_back();
	dirPath.pop_back();
	return;
}

void directoryContents::pushWorkingDir(const std::string& parent) {
	dirPath.append(parent);
	return;
}

std::string directoryContents::getDirPath() {
	return dirPath;
}

uint16_t directoryContents::getDirPathSize() {
	return dirPath.size();
}