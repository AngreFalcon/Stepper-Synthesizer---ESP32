#include "stepper.hpp"
#include "FastAccelStepper.h"
#include "globals.hpp"
#include "midi.hpp"
#include <sstream>
// get rid of annoying library warning
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

FastAccelStepperEngine engine = FastAccelStepperEngine();
std::array<FastAccelStepper*, STEPPER_CHANNELS> stepper;

void initializeStepper(void) {
  engine.init();
  stepper.fill(NULL);
  stepper[0] = engine.stepperConnectToPin(MTR_0_STEP);
  stepper[1] = engine.stepperConnectToPin(MTR_1_STEP);
  stepper[2] = engine.stepperConnectToPin(MTR_2_STEP);
  stepper[3] = engine.stepperConnectToPin(MTR_3_STEP);
  for (uint8_t j = 0; j < STEPPER_CHANNELS; j++) {
    if (stepper[j]) {
      stepper[j]->setAutoEnable(true);
      stepper[j]->setAcceleration(INT_MAX);
      stepper[j]->applySpeedAcceleration();
    }
  }
  return;
}

void playNote(const uint32_t deltaTime, const uint32_t note, const uint8_t event, std::stringstream& debugString) {
  static std::array<uint32_t, 16> prevNote = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  const uint8_t channel = (event & 0x0F);
  delayMicroseconds(deltaTime);
  stepper[0]->setSpeedInMilliHz(note);
  if ((event >> 4) == 9) {
    while (stepper[0]->isStopping());
    stepper[0]->runForward();
    prevNote[channel] = note;
  }
  else if (note == prevNote[channel]) {
    stepper[0]->stopMove();
  }
  debugString << " - DeltaTime in uS: " << deltaTime << " | Event: " << (uint16_t)event << " | Note in mHz: " << note;
  return;
}