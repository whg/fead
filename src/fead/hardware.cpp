#include "hardware.hpp"

#include "Arduino.h"

namespace fead {

typedef void(SerialUnit::*interrupt_callback_t)(uint8_t, uint8_t);

SerialUnit* SerialUnit::sUnits[FEAD_NUM_SERIAL_UNITS] = {
    nullptr, nullptr, nullptr, nullptr
};

void SerialUnit::open(uint8_t number) {
	mUnitNumber = number;

	switch (mUnitNumber) {
	case 0:FEAD_INIT_SERIAL(0); break;
#ifdef UART_1_AVAILABLE
	case 1:FEAD_INIT_SERIAL(1); break;
#endif
#ifdef UART_2_AVAILABLE
	case 2:FEAD_INIT_SERIAL(2); break;
#endif
#ifdef UART_3_AVAILABLE
	case 3:FEAD_INIT_SERIAL(3); break;
#endif			
	default: FEAD_INIT_SERIAL(0); break;
	}

	SerialUnit::sUnits[number] = this;
	sei();

}

void SerialUnit::send(const Packet &packet) const {
	digitalWrite(mDePin, HIGH);
	
	switch (mUnitNumber) {
	case 0: { FEAD_SEND_PACKET(0, packet); break; }
#ifdef UART_1_AVAILABLE
	case 1: { FEAD_SEND_PACKET(1, packet); break; }
#endif
#ifdef UART_2_AVAILABLE
	case 2: { FEAD_SEND_PACKET(2, packet); break; }
#endif
#ifdef UART_3_AVAILABLE
	case 3: { FEAD_SEND_PACKET(3, packet); break; }
#endif			
	default: { FEAD_SEND_PACKET(0, packet); break; }
	}
	
	digitalWrite(mDePin, LOW);
}

void SerialUnit::setDePin(uint8_t pin) {
	mDePin = pin;
	pinMode(mDePin, OUTPUT);
	digitalWrite(mDePin, LOW); // listen
}	
	
}

#if defined(__AVR_ATmega328P__)
ISR(USART_RX_vect) {
#else
ISR(USART0_RX_vect) {
#endif
	// read these first
	uint8_t status = UCSR0A;
	uint8_t data = UDR0;
	if (fead::SerialUnit::sUnits[0]) {
		fead::SerialUnit::sUnits[0]->receive(status, data);
	}
}

#ifdef UART_1_AVAILABLE
ISR(USART1_RX_vect) {
	uint8_t status = UCSR1A;
	uint8_t data = UDR1;
	if (fead::SerialUnit::sUnits[1]) {
		fead::SerialUnit::sUnits[1]->receive(status, data);
	}
}
#endif
