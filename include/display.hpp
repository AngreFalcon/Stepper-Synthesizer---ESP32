#ifndef DISPLAY_HPP
#define DISPLAY_HPP
#include "TFT_eSPI.h"
#include "globals.hpp"
#include "rotary.hpp"

void initializeDisplay(void);                                        // used to get our display ready by setting initial conditions before anything is printed
uint8_t linePadding(uint8_t);                                        // takes as a parameter a line number and then, using our predefined vertical padding, calculates the y value to be passed into tft_espi's drawString() function
void refreshDisplay(void);                                           // performs a per-line update of our display, and also fully redraws the display when redrawDisplay == true
void colorText(const std::string&, uint8_t, bool highlight = false); // called by either our drawDisplay() or refreshDisplay() function, and is used to highlight a single line of text to emulate a cursor for file or folder selection

#endif