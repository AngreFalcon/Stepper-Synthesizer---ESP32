#include "globals.hpp"

volatile uint8_t prevEncoderValue;
uint8_t encoderUpperLimit = 20;

bool redrawDisplay = false;

volatile std::atomic<unsigned long> timeElapsedOld{};
volatile std::atomic<bool> rotaryButtonPressed{};