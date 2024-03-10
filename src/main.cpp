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
  // check to make sure the time since we last received a button press event
  // was at least 250ms. otherwise, ignore the input as potentially unintended
  if (timeElapsedNew - timeElapsedOld > 250) { 
    // either navigate to the new directory location
    // or open the file that the user has selected
    navigateDirectories();       

    // update our variable to now store the new time
    // since our last button press event   
    timeElapsedOld = timeElapsedNew;
  }

  // update our display with our cursor's location if it has changed
  // or redraw the file contents on our display
  // if we have navigated to a different page
  refreshDisplay(); 

  // check if sd card is removed after initialization
  if (!querySD()) {           
    // if sd card was removed, update indicator to reflect sd card status  
    sdInitStatus(false);        

    // attempt to reinitialize sd card
    while (!initializeSDCard()) { 
      // if reinitialization of sd card fails
      // wait 0.5 seconds before trying again
      delay(500);               
    }

    // once reinitialization of sd card succeeds, update indicator
    sdInitStatus(true);         
  }
}