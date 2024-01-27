#pragma warning(push, 0) // disable warnings for libraries
#include "MidiFile.h"
#include "display.hpp"
#include "globals.hpp"
#include "rotary.hpp"
#include "sdio.hpp"
#pragma warning(pop) // re-enable warnings

void setup() {
  // Open serial communications and wait for port to open:
  if (SERIAL_DEBUG) {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();
  }
  pinMode(SD_CARD_INIT_FAILURE, OUTPUT);
  pinMode(SD_CARD_INIT_SUCCESS, OUTPUT);
  pinMode(ROTARY_CLK, INPUT);
  pinMode(ROTARY_DT, INPUT);
  pinMode(ROTARY_SW, INPUT);

  initializeRotary();
  initializeDisplay();
  initializeSDCard();
}

void loop() {
  if (timeElapsedNew - timeElapsedOld > 250) {
    navigateDirectories();
    timeElapsedOld = timeElapsedNew;
  }

  refreshDisplay();
}