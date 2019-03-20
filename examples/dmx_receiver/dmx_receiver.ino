#include <U8g2lib.h>
#include "fead.hpp"

#define USE_DISPLAY

#ifdef USE_DISPLAY
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
#endif

fead::DmxSlave slave;

uint8_t value = 86;
uint16_t bigValue = 23;

void setup() {

#ifdef USE_DISPLAY
  u8g2.begin();
  u8g2.setFont( u8g2_font_9x15_mr );
#endif

  slave.open(0);
  slave.receiveDMX(2, &value);
  slave.receiveDMX(5, &bigValue);
}

void loop() {

  slave.update();
  
#ifdef USE_DISPLAY
  static char buf[12];
  u8g2.clearBuffer();
  itoa(value, buf, 10);
  u8g2.drawStr(0, 10, buf);
  ltoa(bigValue, buf, 10);
  u8g2.drawStr(0, 10 + 15, buf);
  u8g2.sendBuffer();
  delay(200);
#endif

}
