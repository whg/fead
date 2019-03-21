#pragma once

#include <stdint.h>
#include <avr/io.h>

#include "fead/hardware.hpp"
#include "fead/message.hpp"
#include "fead/packet.hpp"
#include "fead/response.hpp"
#include "fead/request.hpp"

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

template <typename T>
using get_handler_t = Response<T> (*)(const Request<T> &req);

template <typename T>
using set_handler_t = bool (*)(const Request<T> &req);

	
enum { FEAD_SLAVE_NO_ADDRESS = 0xff }; //
	
template <typename vocab_t>
class Slave : public SerialUnit {
public:
	Slave(uint8_t address=FEAD_SLAVE_NO_ADDRESS):
		mAddress(address),
		mDmxChannelCounter(0),
		mFeadBufferReady(false),
		mGetHandler(nullptr),
		mSetHandler(nullptr)
	{}
	
	virtual ~Slave() {}
	
	void reply(const Response<vocab_t> &response) {
		send(Packet::create(Command::REPLY, FEAD_MASTER_ADDRESS, response));
	}

	void ack(const Response<vocab_t> &response) {
		send(Packet::create(Command::ACK, FEAD_MASTER_ADDRESS, response));
	}

	template <typename T>
	void receiveDMX(uint16_t channel, T *value) {
		auto *receiver = &mDmxReceivers[mDmxNumReceivers++];
		receiver->channel = channel;
		receiver->numBytes = sizeof(*value);
		receiver->ptr = reinterpret_cast<uint8_t*>(value);
		for (uint8_t i = 0; i < receiver->numBytes; i++) {
			mDmxChannels[mDmxChannelCounter++] = channel + i;
		}
	}

	void setAddress(uint8_t address) {
		mAddress = address;
	}

	void setGetHandler(get_handler_t<vocab_t> handler) {
		mGetHandler = handler;
	}

	void setSetHandler(set_handler_t<vocab_t> handler) {
		mSetHandler = handler;
	}


	void update() {
		uint8_t dmxValueIndex = 0;
		for (uint8_t i = 0; i < mDmxNumReceivers; i++) {
			auto *receiver = &mDmxReceivers[i];
			for (uint16_t j = 0; j < receiver->numBytes; j++) {
				receiver->ptr[j] = mDmxValues[dmxValueIndex++];
			}
		}

		if (mFeadBufferReady) {
			if (mFeadPacket.isValid(mAddress)) {
				uint8_t param = mFeadPacket.bits.param;
				auto request = Request<vocab_t>(param, mFeadPacket.bits.payload);

				switch (mFeadPacket.bits.command) {
				case Command::GET:
					if (mGetHandler) {
						auto response = mGetHandler(request);
						reply(response);
					}
					break;
				case Command::SET:
					if (mSetHandler) {
						auto success = mSetHandler(request);
						if (success) {
							auto response = Response<vocab_t>(param);
							ack(response);
						}
					}
				}
			}

			mFeadBufferReady = false;
		}
	}

public:
	uint8_t thing;
	void receive(uint8_t status, uint8_t data) override {

		if (status & (1<<FE0)) {			
			mByteCounter = 0;
			mPacketType = FEAD_PACKET_TYPE_NONE;
		}
		else {
			if (mByteCounter == 0) {
				mPacketType = data;
				thing = data;
			}
			else if (mPacketType == FEAD_PACKET_TYPE_DMX) {
				for (uint8_t i = 0; i < mDmxChannelCounter; i++) {
					if (mDmxChannels[i] == mByteCounter) {
						mDmxValues[i] = data;						
					}
				}
			}
			
			if (mPacketType == FEAD_PACKET_TYPE_FEAD && !mFeadBufferReady) {

				mFeadPacket.buffer[mByteCounter++] = data;
				if (mByteCounter == FEAD_PACKET_LENGTH) {
					mFeadBufferReady = true;
				}
			} else {
				mByteCounter++;
			}
		}
		
	}


protected:
	uint8_t mSerialUnit;
	uint8_t mAddress;
	
	dmx_receiver_t mDmxReceivers[FEAD_SLAVE_MAX_DMX_RECEIVERS];
	uint8_t mDmxNumReceivers;

	uint16_t mDmxChannels[FEAD_SLAVE_MAX_DMX_CHANNELS];
	volatile uint8_t mDmxValues[FEAD_SLAVE_MAX_DMX_CHANNELS];
	uint8_t mDmxChannelCounter;

	volatile Packet mFeadPacket;
	volatile bool mFeadBufferReady;
	
	volatile uint8_t mByteCounter;
	volatile packet_type_t mPacketType;

protected:
	get_handler_t<vocab_t> mGetHandler;
	set_handler_t<vocab_t> mSetHandler;
};

using DmxSlave = Slave<uint8_t>; // is this good?

}
