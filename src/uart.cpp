#include "uart.hpp"

void initializeUART() {
  Serial1.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);

  return;
}

void checkMIDI() {
  byte commandByte;
  byte noteByte;
  byte velocityByte;
  do {
    if (Serial1.available()) {
      commandByte = Serial1.read();  // read first byte
      noteByte = Serial1.read();     // read next byte
      velocityByte = Serial1.read(); // read final byte
      if (commandByte == 144){//if note on message
        //check if note == 60 and velocity > 0
        if (noteByte == 60 && velocityByte > 0){
          digitalWrite(2, HIGH); //turn on led
        }
      }
    }
  } while (Serial1.available() > 2); // when at least three bytes available
}