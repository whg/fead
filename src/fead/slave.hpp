#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "fead/hardware.hpp"
#include "fead/message.hpp"
#include "fead/packet.hpp"

#ifndef FEAD_SLAVE_MAX_DMX_RECEIVERS
#define FEAD_SLAVE_MAX_DMX_RECEIVERS 8
#endif

namespace fead {

enum dmx_bytes_t { DMX_8BIT, DMX_16BIT };

typedef void (*dmx_8bit_callback_t)(uint8_t);
typedef void (*dmx_16bit_callback_t)(uint16_t);

struct dmx_receiver_t {
	uint16_t channel;
	dmx_8bit_callback_t callback8bit;
	dmx_16bit_callback_t callback16bit;
};
	
template<typename vocab_t>
class Slave : public SerialUnit {
public:
	virtual ~Slave() {}

	Slave(uint8_t serialUnit=0): SerialUnit(serialUnit) {}
	
	void get(uint16_t unit, vocab_t param) {
		send(make_packet(Command::GET, unit, param));
	}

	void set(uint16_t unit, const Message<vocab_t> &msg) {
		send(make_packet(Command::SET, unit, msg));
	}

	void receiveDMX(uint16_t channel, dmx_8bit_callback_t cb) {
		auto *receiver = &mDmxReceivers[mDmxChannelCounter++];
		receiver->channel = channel;
		receiver->callback8bit = cb;
	}

	void receiveDMX(uint16_t channel, dmx_16bit_callback_t cb) {
		auto *receiver = &mDmxReceivers[mDmxChannelCounter++];
		receiver->channel = channel;
		receiver->callback16bit = cb;
	}


protected:
	uint8_t mSerialUnit;
	
	dmx_receiver_t mDmxReceivers[FEAD_SLAVE_MAX_DMX_RECEIVERS];
	uint8_t mDmxChannelCounter;
	
};

}
