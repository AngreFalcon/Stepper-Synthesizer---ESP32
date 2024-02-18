#include "display.hpp"
#include "globals.hpp"
#include "midi.hpp"
#include "oled.hpp"
#include "rotary.hpp"
#include "sdio.hpp"
#include "stepper.hpp"

void setup() {
  // Open serial communications and wait for port to open:
  if (SERIAL_DEBUG) {
    Serial.begin(SERIAL_BAUD_RATE);
    Serial.println();
  }
  initializeRotary();
  initializeDisplay();
  initializeSDIndic(initializeSDCard());
  initializeStepper();
}

void loop() {
  if (timeElapsedNew - timeElapsedOld > 250) { // checks to make sure the time since we last received a button press event was at least 250ms. otherwise, ignore the input as potentially unintended
    navigateDirectories();                     // either navigate to the new directory location or open the file that the user has selected
    timeElapsedOld = timeElapsedNew;           // update our variable to now store the new time since our last button press event
  }
  refreshDisplay(); // update our display with our cursor's location if it has changed, or redraw the file contents on our display if we have navigated to a different page

  if (!querySD()) {             // checks if sd card is removed after initialization
    sdInitStatus(false);        // if sd card was removed, update indicator to reflect sd card status
    while (!initializeSDCard()) { // attempt to reinitialize sd card
      delay(500);               // if reinitialization of sd card fails, wait 1 second before trying again
    }
    sdInitStatus(true);         // once reinitialization of sd card succeeds, update indicator
  }
}