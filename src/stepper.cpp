#include "stepper.hpp"
#include "FastAccelStepper.h"
#include "globals.hpp"
#include "midi.hpp"
// get rid of annoying library warning
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FastAccelStepperEngine engine = FastAccelStepperEngine();
std::array<FastAccelStepper*, STEPPER_CHANNELS> stepper;

void initializeStepper(void) {
  engine.init();
  stepper.fill(NULL);
  stepper[0] = engine.stepperConnectToPin(MTR_STEP);
  if (stepper[0]) {
    stepper[0]->setAutoEnable(true);
    stepper[0]->setAcceleration(INT_MAX);
    stepper[0]->applySpeedAcceleration();
  }
  return;
}

void playNote(const uint32_t deltaTime, const uint32_t note, const uint8_t event) {
  // deltaTime = number of microseconds since last event
  delayMicroseconds(deltaTime);
  while (stepper[0]->isQueueFull());
  stepper[0]->setSpeedInMilliHz(note);
  while (stepper[0]->isQueueFull());
  if ((event & 0xF0) == MIDI_NOTE_ON) {
    stepper[0]->runForward();
  }
  else if ((event & 0xF0) == MIDI_NOTE_OFF) {
    stepper[0]->stopMove();
  }
  return;
}