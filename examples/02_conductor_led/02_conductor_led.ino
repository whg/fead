#include "fead.hpp"

enum vocab {
  LED,
};

fead::Conductor<vocab> conductor;

void setup() {
  conductor.init(1);
}

void loop() {
  conductor.update();
}
