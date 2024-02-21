#ifndef STEPPER_HPP
#define STEPPER_HPP
#include "globals.hpp"

// this macro defines the maximum number of channels (stepper motors)
// connected to our microprocessor
#define STEPPER_CHANNELS 4 

// this function initializes our stepper motor object and sets default parameters
void initializeStepper(void); 

// this function takes in our deltaTime in microseconds,
// note as frequency in millihertz, and event as an 8 bit integer
// and plays a note at the given frequency
// the deltaTime provided dictates the duration of the previously provided
// note; upon calling this function, the note passed in will continue 
// to sound until a note off event occurs or a new frequency is provided
void playNote(const uint32_t deltaTime, const uint32_t note, const uint8_t event);

#endif