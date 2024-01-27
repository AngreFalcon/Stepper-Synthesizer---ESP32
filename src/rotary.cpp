#include "rotary.hpp"
// much of handleEncoder() and callBack() functions are thanks to the examples provided by the NewEncoder library. credit to gfvalvo on GitHub, the author of the library

QueueHandle_t encoderQueue;
NewEncoder* encoder;

bool initializeRotary() {
  BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
  if (!success) {
    if (SERIAL_DEBUG)
      printf("Failed to create handleEncoder task. Aborting.\n");
    return success;
  }
  attachInterrupt(ROTARY_SW, rotaryButton, RISING);
  return success;
}

void handleEncoder(void* pvParameters) {
  NewEncoder::EncoderState currentEncoderstate;
  int16_t currentValue;

  encoderQueue = xQueueCreate(1, sizeof(NewEncoder::EncoderState));
  if (encoderQueue == nullptr) {
    if (SERIAL_DEBUG)
      Serial.println("Failed to create encoderQueue. Aborting");
    vTaskDelete(nullptr);
  } // this conditional ensures that the encoder queue initialized successfully

  encoder = new NewEncoder(ROTARY_CLK, ROTARY_DT, encoderLowerLimit, encoderUpperLimit, encoderLowerLimit, FULL_PULSE);
  if (encoder == nullptr) {
    if (SERIAL_DEBUG)
      Serial.println("Failed to allocate NewEncoder object. Aborting.");
    vTaskDelete(nullptr);
  } // this conditional ensures that the encoder object allocated successfully

  if (!encoder->begin()) {
    if (SERIAL_DEBUG)
      Serial.println("Encoder failed to start. Check pin assignments and available interrupts. Aborting.");
    delete encoder;
    vTaskDelete(nullptr);
  } // this conditional ensures that the encoder object started successfully

  encoder->getState(currentEncoderstate);
  prevEncoderValue = currentEncoderstate.currentValue;
  if (SERIAL_DEBUG)
    Serial.println("Encoder successfully started at value = " + prevEncoderValue);
  encoder->attachCallback(callBack);

  for (;;) {
    xQueueReceive(encoderQueue, &currentEncoderstate, portMAX_DELAY);
    if (SERIAL_DEBUG)
      Serial.print("Encoder: ");
    currentValue = currentEncoderstate.currentValue;
    if (currentValue != prevEncoderValue) {
      if (SERIAL_DEBUG)
        Serial.print(currentValue);
      if (prevEncoderValue / DISPLAY_LINES_PER_SCREEN != currentValue / DISPLAY_LINES_PER_SCREEN)
        redrawDisplay = true;
      prevEncoderValue = currentValue;
    }
    else { // print to serial monitor, if debug macro is true, whether our encoder has hit its upper or lower limit
      if (SERIAL_DEBUG)
        switch (currentEncoderstate.currentClick) {
        case NewEncoder::UpClick:
          //Serial.println("at upper limit.");
          break;
        case NewEncoder::DownClick:
          //Serial.println("at lower limit.");
          break;
        default:
          break;
        }
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

void IRAM_ATTR rotaryButton() {
  timeElapsedNew = millis();
  return;
}

bool updateSettings() {
  NewEncoder::EncoderState state;
  prevEncoderValue = 0;
  return encoder->newSettings(encoderLowerLimit, encoderUpperLimit - 1, encoderLowerLimit, state);
}

uint8_t upOrDown() { // currently not used
  NewEncoder::EncoderState state;
  encoder->getState(state);
  return state.currentClick;
}