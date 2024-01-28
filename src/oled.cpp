#include "oled.hpp"

Adafruit_SSD1306 display(SD_INDIC_WIDTH, SD_INDIC_HEIGHT, &Wire, SD_INDIC_RESET);

void initializeSDIndic(bool sdInit) {
  Wire.begin(SD_INDIC_SDA, SD_INDIC_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, SD_INDIC_ADDRESS) && SERIAL_DEBUG)
    Serial.println(F("SSD1306 allocation failed"));

display.setRotation(3);
  display.clearDisplay();              // clear any data that may be present in our buffer before attempting to draw to our OLED
  display.setTextSize(2);              // due to the small size of our panel, apply 2x multiplier to ensure text is readable
  display.setTextColor(SSD1306_WHITE); // must set text color, otherwise our text, should we attempt to display any, will not show up on our OLED
  display.cp437(true);                 // Use full 256 char 'Code Page 437' font

  sdInitStatus(sdInit);
  return;
}

void sdInitStatus(bool success) {
  if (success)
    display.drawXBitmap(0, 32, sdSuccessIcon, 64, 64, SSD1306_WHITE);
  else {
		display.clearDisplay();
    display.drawXBitmap(0, 32, sdFailureIcon, 64, 64, SSD1306_WHITE);
	}
  display.display();
  return;
}