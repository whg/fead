#pragma once

#include <stdint.h>

#include "fead/hardware.hpp"
#include "fead/request.hpp"
#include "fead/packet.hpp"

#include "fead/debug.hpp"

namespace fead {

template<typename param_t>
class Master : public SerialUnit {
public:
	Master() = default;
			
	virtual ~Master() {}
	
	void get(uint16_t unit, const Request<param_t> &req) {
		send(make_packet(Command::GET, unit, req));
	}

	void set(uint16_t unit, const Request<param_t> &req) {
		send(make_packet(Command::SET, unit, req));
	}

	void receive(uint8_t status, uint8_t data) override {
		PORTB |= (1<<PB7);
	}

	void print() {
		Debug.print("master: ");
		Debug.println(int(this));
	}

protected:
	
	
};
	
}
