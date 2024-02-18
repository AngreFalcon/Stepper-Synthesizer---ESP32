#ifndef OLED_HPP
#define OLED_HPP

void initializeSDIndic(bool sdInit); // used to set default parameters for our SD card indicator, such as orientation and font size, as well as initialize the I2C object used to communicate with our OLED
void sdInitStatus(bool success);     // prints either a success or fail icon to our OLED depending on the boolean passed in as an argument

#endif