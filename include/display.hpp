#ifndef DISPLAY_HPP
#define DISPLAY_HPP
#include <TFT_eSPI.h>

// used to get our display ready by setting initial conditions
// before anything is printed
void initializeDisplay(void);                                                                          

// takes as a parameter a line number and then
// using our predefined vertical padding
// calculates the y value to be passed into tft_espi's drawString() function
uint8_t linePadding(const uint8_t index);            

// performs a per-line update of our display
// and also fully redraws the display when redrawDisplay == true
void refreshDisplay(void);                                                                             

// called by either our drawDisplay() or refreshDisplay() function
// and is used to highlight a single line of text
// to emulate a cursor for file or folder selection
void colorText(const std::string& printString, const uint8_t verticalPadding, bool highlight = false); 

#endif