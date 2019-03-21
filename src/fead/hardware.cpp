#include "hardware.hpp"

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
	// read these first
	uint8_t status = UCSR0A;
	uint8_t data = UDR0;
	fead::SerialUnit::sUnits[0]->receive(status, data);
}

#ifdef UART_1_AVAILABLE
ISR(USART1_RX_vect) {
	uint8_t status = UCSR1A;
	uint8_t data = UDR1;
	fead::SerialUnit::sUnits[1]->receive(status, data);
}
#endif
