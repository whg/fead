#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "fead/hardware.hpp"
#include "fead/message.hpp"
#include "fead/packet.hpp"

#ifndef FEAD_SLAVE_MAX_DMX_RECEIVERS
#define FEAD_SLAVE_MAX_DMX_RECEIVERS 5
#endif

#ifndef FEAD_SLAVE_MAX_DMX_CHANNELS
#define FEAD_SLAVE_MAX_DMX_CHANNELS 8
#endif

namespace fead {

struct dmx_receiver_t {
	uint16_t channel;
	uint8_t numBytes;
	uint8_t *ptr;
};
	
template<typename vocab_t>
class Slave : public SerialUnit {
public:
	Slave(): mDmxChannelCounter(0) {}
	
	virtual ~Slave() {}
	
	void get(uint16_t unit, vocab_t param) {
		send(make_packet(Command::GET, unit, param));
	}

	void set(uint16_t unit, const Message<vocab_t> &msg) {
		send(make_packet(Command::SET, unit, msg));
	}

	template<typename T>
	void receiveDMX(uint16_t channel, T *value) {
		auto *receiver = &mDmxReceivers[mDmxNumReceivers++];
		receiver->channel = channel;
		receiver->numBytes = sizeof(*value);
		receiver->ptr = reinterpret_cast<uint8_t*>(value);
		for (uint8_t i = 0; i < receiver->numBytes; i++) {
			mDmxChannels[mDmxChannelCounter++] = channel + i;
		}
	}

	void update() {
		uint8_t dmxValueIndex = 0;
		for (uint8_t i = 0; i < mDmxNumReceivers; i++) {
			auto *receiver = &mDmxReceivers[i];
			for (uint16_t j = 0; j < receiver->numBytes; j++) {
				receiver->ptr[j] = mDmxValues[dmxValueIndex++];
			}
		}
	}

public:
	void receive(uint8_t status, uint8_t data) override {

		if (status & (1<<FE0)) {			
			mReceivedByteCounter = 0;
			mPacketType = FEAD_PACKET_TYPE_NONE;
		}
		else {
			if (mReceivedByteCounter == 0) {
				mPacketType = data;
			}
			else if (mPacketType == FEAD_PACKET_TYPE_DMX) {
				for (uint8_t i = 0; i < mDmxChannelCounter; i++) {
					if (mDmxChannels[i] == mReceivedByteCounter) {
						mDmxValues[i] = data;						
					}
				}
			}
			mReceivedByteCounter++;
		}
		
	}


protected:
	uint8_t mSerialUnit;
	
	dmx_receiver_t mDmxReceivers[FEAD_SLAVE_MAX_DMX_RECEIVERS];
	uint8_t mDmxNumReceivers;

	uint16_t mDmxChannels[FEAD_SLAVE_MAX_DMX_CHANNELS];
	volatile uint8_t mDmxValues[FEAD_SLAVE_MAX_DMX_CHANNELS];
	uint8_t mDmxChannelCounter;

	volatile uint8_t mReceivedByteCounter;
	volatile packet_type_t mPacketType;
};

using DmxSlave = Slave<uint8_t>; // is this good?

}
