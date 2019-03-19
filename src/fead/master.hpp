#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "fead/message.hpp"
#include "fead/packet.hpp"

#include "fead/debug.hpp"

namespace fead {

template<typename param_t>
class Master {
public:
	virtual ~Master() {}

	Master() {
		UBRR1L = 3; // 250k baud
		UCSR1B = (1<<RXEN1) | (1<<TXEN1) | (1<<RXCIE1);
		UCSR1C = (1<<USBS1) | (1<<UCSZ11) | (1<<UCSZ10); // 8 bit, 2 stop bits
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


}
