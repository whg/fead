#pragma once

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "fead/packet.hpp"
#include "fead/debug.hpp"

#define FEAD_FULL_SPEED 3 // 250k baud at 16MHz
#define FEAD_HALF_SPEED 8 // 115.2k baud at 16MHz

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

// oddly, you have to write a 1 to the TXC flag in order to clear it

#define FEAD_SEND_PACKET(n, packet)							   \
	cli();													   \
	FEAD_SET_SERIAL_SPEED(n, FEAD_HALF_SPEED);				   \
	UDR##n = 0;												   \
	UCSR##n##A |= (1<<TXC##n);								   \
	loop_until_bit_is_set(UCSR##n##A, TXC##n);				   \
	FEAD_SET_SERIAL_SPEED(n, FEAD_FULL_SPEED);				   \
	uint8_t *ptr = &packet.buffer[0];						   \
	for (uint8_t i = 0; i < FEAD_PACKET_LENGTH; i++) {		   \
		loop_until_bit_is_set(UCSR##n##A, UDRE##n);			   \
		UDR##n = *ptr++;									   \
	}														   \
	UCSR##n##A |= (1<<TXC##n);								   \
	loop_until_bit_is_set(UCSR##n##A, TXC##n);				   \
	sei();


namespace fead {

class SerialUnit {
public:
	SerialUnit();
	virtual ~SerialUnit() = default;

	virtual void open(uint8_t number);
	void send(const Packet &packet) const;

	virtual void setDePin(uint8_t pin);
	virtual void driverEnable(bool on=true);

public:
	virtual void receive(uint8_t status, uint8_t data) = 0;

public:
	static SerialUnit *sUnits[4];

protected:
	uint8_t mUnitNumber;
	uint8_t mDePin;
};

}
