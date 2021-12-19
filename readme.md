# fead
## a communication library

**fead** is a DMX compatible Arduino library to send and receive data on an RS485 bus.


## Writing the UID

If clients on a fead network don't have their address explicitly set, they use their UID as their default address. All fead clients should have their UID set before being used.

You can write the UID to EEPROM using the `create-uid-hex` helper script and `avrdude` like:

```sh
./scripts/create-uid-hex.sh 12
avrdude -v -patmega328p -catmelice_isp -Pusb -U eeprom:w:uid-eeprom.hex
```

which writes a UID of 12. Alternatively, you can flash a program to write the slot:

```cpp
#include "fead.hpp"

void setup() {
  EEPROM.put(fead::EEPROMSlot::UID, 12);
}

void loop() {}
```

which also writes a UID of 12.
