#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "fead/message.hpp"
#include "fead/packet.hpp"

namespace fead {

template<typename param_t>
class Slave {
public:
	virtual ~Slave() {}

	Slave() {
		UBRR0L = 3; // 250k baud
		UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1<<RXCIE0);
		UCSR0C = (1<<USBS0) | (1<<UCSZ01) | (1<<UCSZ00); // 8 bit, 2 stop bits
	}
	
	void get(uint16_t unit, param_t param) {
		send(make_packet(Command::GET, unit, param));
	}

	void set(uint16_t unit, const Message<param_t> &msg) {
		send(make_packet(Command::SET, unit, msg));
	}

protected:
	void send(const packet_t &packet) {
		for (uint8_t i = 0; i < FEAD_PACKET_LENGTH; i++) {
			Debug.println(int(packet.buffer[i]));
			// TODO: send out of UART1
		}
	}
	
};

using EasySlave = Slave<int>;
}
