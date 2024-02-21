#ifndef OLED_HPP
#define OLED_HPP

// used to set default parameters for our SD card indicator
// such as orientation and font size, as well as initialize the
// I2C object used to communicate with our OLED
void initializeSDIndic(bool sdInit); 

// prints either a success or fail icon to our OLED
// depending on the boolean passed in as an argument
void sdInitStatus(bool success);     

#endif