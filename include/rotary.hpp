#ifndef ROTARY_H
#define ROTARY_H
#include "globals.hpp"

extern QueueHandle_t encoderQueue; // the rotary encoder library uses this object to keep track of inputs in between interrupts so that information is not lost during rapid actuation of our encoder
extern NewEncoder* encoder;        // this is the object that references our rotary encoder and is used for tracking attributes such as direction of input

bool initializeRotary();
void handleEncoder(void* pvParameters);
void ESP_ISR callBack(NewEncoder* encPtr, const volatile NewEncoder::EncoderState* state, void* uPtr);
void IRAM_ATTR rotaryButton(); // here is our interrupt for tracking button clicks of our rotary encoder
bool updateSettings();         // wrapper function to simplify updating the maximum value of our encoder object
uint8_t upOrDown();            // a return value of 1 means down, 2 means up

#endif