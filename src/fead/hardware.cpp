#include "hardware.hpp"

#include <avr/interrupt.h>

namespace fead {

typedef void(SerialUnit::*interrupt_callback_t)(uint8_t, uint8_t);

SerialUnit* SerialUnit::sUnits[FEAD_NUM_SERIAL_UNITS] = {
    nullptr, nullptr, nullptr, nullptr
};

}

#if defined(__AVR_ATmega328P__)
ISR(USART_RX_vect) {
#else
ISR(USART0_RX_vect) {
#endif
	fead::SerialUnit::sUnits[0]->receive(UCSR0A, UDR0);
}

#if defined(UBRR1L) 
ISR(USART1_RX_vect) {
	// uint8_t status = UCSR1A;
	// uint8_t d = UDR1;
	fead::SerialUnit::sUnits[1]->receive(UCSR0A, UDR0);
}
#endif
