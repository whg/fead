#include <EEPROM.h>

//////////////////////////////////////////////////
uint8_t uid = 1;
uint8_t address = 1;
//////////////////////////////////////////////////

namespace fead {

enum EEPROMSlot {
  UID = 200,
  ADDRESS = 201,
};

}


void setup() {
  EEPROM.put(fead::EEPROMSlot::UID, uid);  
  EEPROM.put(fead::EEPROMSlot::ADDRESS, address);  
}

void loop() {}
