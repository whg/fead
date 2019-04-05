#include "fead.hpp"
#include <EEPROM.h>
#include <avr/wdt.h>

enum vocab {
  LED,
  LED2,
  TEMP,
  setAddress,
};

#include "BubbleTree.hpp"

fead::Slave<vocab> slave;

bool ledState = false;

BubbleTree tree(LED_BUILTIN, 10, A0);


void setup() {

  slave.open(0);
  slave.setAddress(EEPROM.read(0));
  slave.setDePin(2);
  slave.setHandler(&tree);
}

void loop() {
  slave.update();
  tree.update();
}

