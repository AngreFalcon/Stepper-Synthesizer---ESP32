#include "stepper.hpp"

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper* stepper = NULL;

void initializeStepper(void) {
  engine.init();
  stepper = engine.stepperConnectToPin(MTR_STEP);
  if (stepper) {
    stepper->setAutoEnable(true);
    stepper->setSpeedInHz(midi::getFreq("C", 3));
    stepper->setAcceleration(INT_MAX);
    stepper->applySpeedAcceleration();
    stepper->runForward();
  }
  return;
}