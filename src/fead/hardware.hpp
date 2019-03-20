#pragma once

#include <avr/io.h>

#include "fead/packet.hpp"
#include "fead/debug.hpp"

#define FEAD_FULL_SPEED 3
#define FEAD_HALF_SPEED 8

#define FEAD_NUM_SERIAL_UNITS 4

#if defined(UCSR1A)
#define UART_1_AVAILABLE
#endif

#if defined(UCSR2A)
#define UART_2_AVAILABLE
#endif

#if defined(UCSR3A)
#define UART_3_AVAILABLE
#endif

#define FEAD_SET_SERIAL_SPEED(n, speed) UBRR##n##L = speed

// init serial to 250k baud, 8 bit data, 2 stop bits
// enable tx & rx and rx interrupt
#define FEAD_INIT_SERIAL(n)												\
	FEAD_SET_SERIAL_SPEED(n, FEAD_FULL_SPEED);							\
	UCSR##n##B = (1<<RXEN##n) | (1<<TXEN##n) | (1<<RXCIE##n);			\
	UCSR##n##C = (1<<USBS##n) | (1<<UCSZ##n##1) | (1<<UCSZ##n##0)		\


#define FEAD_SEND_PACKET(n, packet)							   \
		FEAD_SET_SERIAL_SPEED(n, FEAD_HALF_SPEED);			   \
		loop_until_bit_is_set(UCSR##n##A, UDRE##n);			   \
		UDR##n = 0;											   \
		FEAD_SET_SERIAL_SPEED(n, FEAD_FULL_SPEED);			   \
		uint8_t *ptr = &packet.buffer[0];					   \
		for (uint8_t i = 0; i < FEAD_PACKET_LENGTH; i++) {	   \
			loop_until_bit_is_set(UCSR##n##A, UDRE##n);		   \
			UDR##n = *ptr++;								   \
		}

namespace fead {
	
class SerialUnit {
public:
	SerialUnit() = default;
	virtual ~SerialUnit() = default;
	
	void open(uint8_t number) {
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

	}

	void send(const packet_t &packet) const {
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
	}

	virtual void receive(uint8_t status, uint8_t data) = 0;

	static SerialUnit *sUnits[4];

protected:
	uint8_t mUnitNumber;
};

}

