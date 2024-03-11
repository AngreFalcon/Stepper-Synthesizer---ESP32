#include "rotary.hpp"
#include "globals.hpp"
// much of handleEncoder() and callBack() functions are thanks
// to the examples provided by the NewEncoder library
// credit to gfvalvo on GitHub, the author of the library

// the rotary encoder library uses this object to keep track of inputs
// in between interrupts so that information is not lost during rapid actuation
// of our encoder
QueueHandle_t encoderQueue;

// this is the object that references our rotary encoder
// and is used for tracking attributes such as direction of input
NewEncoder* encoder;

// here we define the lower limit of our encoder value
// as a default, this will be zero; once hitting the upper limit
// we should cycle around to the lower limit
const uint8_t encoderLowerLimit = 0;

bool initializeRotary(void) {
  BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
  if (!success) {
    if (SERIAL_DEBUG) {
      printf("Failed to create handleEncoder task. Aborting.\n");
    }
    return success;
  }
  pinMode(ROTARY_CLK, INPUT);
  pinMode(ROTARY_DT, INPUT);
  pinMode(ROTARY_SW, INPUT);
  attachInterrupt(ROTARY_SW, rotaryButton, RISING);
  return success;
}

void handleEncoder(void* pvParameters) {
  NewEncoder::EncoderState currentEncoderstate;
  int16_t currentValue;
  encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));

  // this conditional ensures that the encoder queue initialized successfully
  if (encoderQueue == nullptr) {
    if (SERIAL_DEBUG) {
      Serial.println("Failed to create encoderQueue. Aborting");
    }
    vTaskDelete(nullptr);
  }
  encoder = new NewEncoder(ROTARY_CLK, ROTARY_DT, encoderLowerLimit, encoderUpperLimit, encoderLowerLimit, FULL_PULSE);

  // this conditional ensures that the encoder object allocated successfully
  if (encoder == nullptr) {
    if (SERIAL_DEBUG) {
      Serial.println("Failed to allocate NewEncoder object. Aborting.");
    }
    vTaskDelete(nullptr);
  }

  // this conditional ensures that the encoder object started successfully
  if (!encoder->begin()) {
    if (SERIAL_DEBUG) {
      Serial.println("Encoder failed to start. Check pin assignments and available interrupts. Aborting.");
    }
    delete encoder;
    vTaskDelete(nullptr);
  }

  encoder->getState(currentEncoderstate);
  prevEncoderValue = currentEncoderstate.currentValue;
  if (SERIAL_DEBUG) {
    Serial.print("Encoder successfully started at value = ");
  }
  Serial.println(prevEncoderValue);
  encoder->attachCallback(callBack);

  for (;;) {
    xQueueReceive(encoderQueue, &currentEncoderstate, portMAX_DELAY);
    currentValue = currentEncoderstate.currentValue;
    timeElapsedOld = millis();
    rotaryButtonPressed = false;
    if (currentValue != prevEncoderValue) {
      if (prevEncoderValue / DISPLAY_LINES_PER_SCREEN != currentValue / DISPLAY_LINES_PER_SCREEN) {
        redrawDisplay = true;
      }
      prevEncoderValue = currentValue;
    }
  }
  vTaskDelete(nullptr);
}

void ESP_ISR callBack(NewEncoder* encPtr, const volatile NewEncoder::EncoderState* state, void* uPtr) {
  BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
  xQueueOverwriteFromISR(encoderQueue, (void*)state, &pxHigherPriorityTaskWoken);
  if (pxHigherPriorityTaskWoken) {
    portYIELD_FROM_ISR();
  }
}

void IRAM_ATTR rotaryButton(void) {
  if (millis() - timeElapsedOld > 250 && !rotaryButtonPressed) {
    rotaryButtonPressed = true;
  }
  return;
}

bool updateSettings(void) {
  NewEncoder::EncoderState state;
  prevEncoderValue = 0;
  return encoder->newSettings(encoderLowerLimit, encoderUpperLimit - 1, encoderLowerLimit, state);
}

// currently not used
uint8_t upOrDown(void) {
  NewEncoder::EncoderState state;
  encoder->getState(state);
  return state.currentClick;
}