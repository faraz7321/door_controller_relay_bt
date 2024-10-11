#define IR_SEND_PIN 3

#include <Arduino.h>
#include "PinDefinitionsAndMore.h"
#include <IRremote.hpp>

const int buttonPin = 10;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(buttonPin, INPUT); // Initialization sensor pin
  digitalWrite(buttonPin, HIGH); // Activation of internal pull-up resistor

  Serial.begin(115200);

  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));

  /*
     The IR library setup. That's all!
  */
  IrSender.begin();
  Serial.print(F("Ready to send IR signals at pin "));
  Serial.println(IR_SEND_PIN);
}


uint16_t sAddress = 0x0102;
uint8_t sCommand = 0x34;

uint8_t sRepeats = 0;

void loop() {

  if(digitalRead(buttonPin) == LOW ){

  Serial.println(F("Send NEC with 16 bit address"));
  Serial.flush();

  IrSender.sendNEC(sAddress, sCommand, sRepeats);
  delay(1000);
  }
}
