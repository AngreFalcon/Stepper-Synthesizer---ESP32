#ifndef STEPPER_HPP
#define STEPPER_HPP
#include "globals.hpp"

#define STEPPER_CHANNELS 4 // this macro defines the maximum number of channels (stepper motors) connected to our microprocessor

void initializeStepper(void); // this function initializes our stepper motor object and sets default parameters
void playNote(const uint32_t deltaTime, const uint8_t note, const uint8_t event);

#endif