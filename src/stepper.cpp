#include "stepper.hpp"
#include "FastAccelStepper.h"
#include "globals.hpp"
#include "midi.hpp"
#include <sstream>
// get rid of annoying library warning
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

struct NoteChannel {
  uint32_t note = 0;
  uint8_t channel = NO_CHANNEL;
};

FastAccelStepperEngine engine = FastAccelStepperEngine();
std::array<FastAccelStepper*, STEPPER_CHANNELS> stepper;

void initializeStepper(void) {
  const std::array<uint8_t, STEPPER_CHANNELS> mtrSteps = { MTR_STEPS };
  engine.init();
  stepper.fill(NULL);
  for (uint8_t j = 0; j < STEPPER_CHANNELS; j++) {
    stepper[j] = engine.stepperConnectToPin(mtrSteps[j]);
    stepper[j]->setAutoEnable(true);
    stepper[j]->setAcceleration(INT_MAX);
    stepper[j]->applySpeedAcceleration();
  }
  return;
}

uint8_t findStepperPreferPoly(const std::array<NoteChannel, STEPPER_CHANNELS>& activeChannels, const uint8_t channel) {
  uint8_t stepperOnChannel = NOT_FOUND;
  for (uint8_t i = 0; i < STEPPER_CHANNELS; ++i) {
    if (activeChannels[i].channel == NO_CHANNEL) {
      // prefer first available stepper
      return i;
    }
    if (activeChannels[i].channel == channel && stepperOnChannel == NOT_FOUND) {
      // use already active stepper as a fallback, if one is active for this channel
      stepperOnChannel = i;
    }
  }
  return stepperOnChannel;
}

uint8_t findStepperPreferMono(const std::array<NoteChannel, STEPPER_CHANNELS>& activeChannels, const uint8_t channel) {
  uint8_t firstFound = NOT_FOUND;
  for (uint8_t i = 0; i < STEPPER_CHANNELS; ++i) {
    if (activeChannels[i].channel == channel) {
      // prefer taking over an already active stepper on this channel
      return i;
    }
    if (activeChannels[i].channel == NO_CHANNEL && firstFound == NOT_FOUND) {
      // use the first available stepper as a fallback
      firstFound = i;
    }
  }
  return firstFound;
}

uint8_t findActiveStepper(const std::array<NoteChannel, STEPPER_CHANNELS>& activeChannels, const uint32_t note, const uint8_t channel) {
  for (uint8_t i = 0; i < STEPPER_CHANNELS; ++i) {
    if (activeChannels[i].channel == channel && activeChannels[i].note == note) {
      return i;
    }
  }
  return NOT_FOUND;
}

void playNote(const uint32_t deltaTime, const uint32_t note, const uint8_t event, const bool polyphonic, std::stringstream& debugString) {
  const uint8_t channel = (event & 0x0F), eventType = (event >> 4);
  static std::array<NoteChannel, STEPPER_CHANNELS> activeChannels {};
  delayMicroseconds(deltaTime);

  uint8_t stepperIdx = NOT_FOUND;
  if (eventType == MIDI_NOTE_ON >> 4) {
    stepperIdx = polyphonic ? findStepperPreferPoly(activeChannels, channel) : findStepperPreferMono(activeChannels, channel);
    if (stepperIdx != NOT_FOUND) {
      activeChannels[stepperIdx].channel = channel;
      activeChannels[stepperIdx].note = note;
      stepper[stepperIdx]->setSpeedInMilliHz(note);
      stepper[stepperIdx]->runForward();
    }
  }
  else {
    stepperIdx = findActiveStepper(activeChannels, note, channel);
    if (stepperIdx != NOT_FOUND) {
      activeChannels[stepperIdx].channel = NO_CHANNEL;
      stepper[stepperIdx]->stopMove();
    }
  }

  debugString << " - DeltaTime in uS: " << deltaTime << " | Event: " << (uint16_t)event << " | Note in mHz: " << note << " | Stepper Index: " << std::to_string(stepperIdx);
  return;
}