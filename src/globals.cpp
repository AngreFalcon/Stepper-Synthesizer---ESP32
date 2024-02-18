#include "globals.hpp"

volatile uint8_t prevEncoderValue;
uint8_t encoderUpperLimit = 20;

bool redrawDisplay = false;
unsigned long timeElapsedOld = 0;
unsigned long timeElapsedNew = 0;