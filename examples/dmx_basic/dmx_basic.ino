#include "fead.hpp"

fead::DmxSlave slave;

uint8_t ledPin = 5;

// declare a variable to control via DMX
uint8_t ledBrightness;

void setup() {
  // open the slave on serial 0
  slave.open(0);

  // receive DMX on channel 2
  // we pass a reference to ledBrightness, so it's value is changed for us when we receive DMX
  slave.receiveDMX(2, &ledBrightness);
  
  pinMode(ledPin, 13);
}

void loop() {
  slave.update();

  analogWrite(ledPin, ledBrightness);
}
