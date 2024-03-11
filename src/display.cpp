#include "display.hpp"
#include "globals.hpp"
#include "rotary.hpp"
#include "sdio-directoryContents.hpp"

// this is the object that tft_espi uses to interface with our display
TFT_eSPI tft = TFT_eSPI();

void initializeDisplay(void) {
  tft.begin();
  tft.setRotation(1);
  tft.setTextDatum(TL_DATUM);
  tft.setTextPadding(DISPLAY_HORIZONTAL_PADDING);
  tft.setTextColor(DISPLAY_TEXT, DISPLAY_BG);
  tft.fillScreen(DISPLAY_BG);
  if (SERIAL_DEBUG) {
    Serial.println("Display successfully initialized.");
  }
}

uint8_t linePadding(const uint8_t index) {
  return (index * DISPLAY_FONT_HEIGHT + (index * DISPLAY_FONT_VERTICAL_PADDING));
}

void refreshDisplay(void) {
  // must lock value to avoid race condition;
  // attempting to access prevEncoderValue appears to
  // on occasion result in crash due to memory access violation
  uint8_t lockedEncoderValue = prevEncoderValue;

  // we use this value to find the index offset
  // needed to print our text to the screen
  uint8_t page = (lockedEncoderValue / DISPLAY_LINES_PER_SCREEN) * DISPLAY_LINES_PER_SCREEN;

  // make sure we don't attempt to update the display
  // if there are no directory contents to display
  // this is to ensure we do not accidentally print directory contents
  // that do not actually exist
  if (myDir.contents.size() < 2 && !redrawDisplay) {
    return;
  }
  if (redrawDisplay) {
    tft.fillScreen(DISPLAY_BG);
  }
  for (uint8_t i = 0; i < DISPLAY_LINES_PER_SCREEN && (i + page) < encoderUpperLimit; i++) {
    if ((i + page) != lockedEncoderValue && (myDir.highlighted[i] || redrawDisplay)) {
      if (i + page <= myDir.containedDirs) {
        tft.drawString(myDir.contents[i + page].c_str(), DISPLAY_FONT_HORIZONTAL_PADDING, linePadding(i), DISPLAY_FONT);
      }
      else if (i + page < myDir.contents.size()) {
        colorText(myDir.contents[i + page], linePadding(i), false);
      }
      myDir.highlighted[i] = false;
    }
    else if ((i + page) == lockedEncoderValue && (!myDir.highlighted[i] || redrawDisplay)) {
      colorText(myDir.contents[i + page], linePadding(i), true);
      myDir.highlighted[i] = true;
    }
  }
  redrawDisplay = false;
  return;
}

void colorText(const std::string& printString, const uint8_t verticalPadding, const bool highlight) {
  if (highlight) {
    tft.setTextColor(DISPLAY_TEXT_HL, DISPLAY_HL);
    tft.setTextPadding(0);
  }
  else {
    tft.setTextColor(DISPLAY_HL, DISPLAY_BG);
  }
  tft.drawString((printString + " ").c_str(), DISPLAY_FONT_HORIZONTAL_PADDING, verticalPadding, DISPLAY_FONT);
  tft.setTextColor(DISPLAY_TEXT, DISPLAY_BG);
  tft.setTextPadding(DISPLAY_HORIZONTAL_PADDING);
  return;
}