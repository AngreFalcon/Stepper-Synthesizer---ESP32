#ifndef ROTARY_H
#define ROTARY_H
#include "NewEncoder.h"

bool initializeRotary(void); // used to configure parameters for our rotary encoder such as upper and lower limits, as well as attach our interrupt to handle rotary encoder events
void handleEncoder(void* pvParameters);
void ESP_ISR callBack(NewEncoder* encPtr, const volatile NewEncoder::EncoderState* state, void* uPtr);
void IRAM_ATTR rotaryButton(void); // here is our interrupt for tracking button clicks of our rotary encoder
bool updateSettings(void);         // wrapper function to simplify updating the maximum value of our encoder object
uint8_t upOrDown(void);            // a return value of 1 means down, 2 means up

#endif