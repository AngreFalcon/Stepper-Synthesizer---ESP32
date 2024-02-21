#ifndef ROTARY_H
#define ROTARY_H
#include "NewEncoder.h"

// used to configure parameters for our rotary encoder such as
// upper and lower limits, as well as attach our interrupt
// to handle rotary encoder events
bool initializeRotary(void); 

void handleEncoder(void* pvParameters);

void ESP_ISR callBack(NewEncoder* encPtr, const volatile NewEncoder::EncoderState* state, void* uPtr);

// here is our interrupt for tracking button clicks of our rotary encoder
void IRAM_ATTR rotaryButton(void); 

// wrapper function to simplify updating the maximum value
// of our encoder object
bool updateSettings(void);        

// a return value of 1 means down, 2 means up 
uint8_t upOrDown(void);            

#endif