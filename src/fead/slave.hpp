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

#define FEAD_SLAVE_UNASSIGNED_ADDRESS 0xff

namespace fead {

struct dmx_receiver_t {
	uint16_t channel;
	uint8_t numBytes;
	uint8_t *ptr;
};


template <typename vocab_t>
class Slave : public SerialUnit {
public:
	class RequestHandler {
	public:
		virtual Response<vocab_t> get(const Request<vocab_t> &req) = 0;
		virtual bool set(const Request<vocab_t> &req) = 0;
	};
	
public:
	Slave(uint8_t address=FEAD_SLAVE_UNASSIGNED_ADDRESS):
		mAddress(address),
		mDmxChannelCounter(0),
		mFeadBufferReady(false),
		mRequestHandler(nullptr)
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

	void setHandler(RequestHandler* const handler) {
		mRequestHandler = handler;
	}

	void update() {

		uint8_t dmxValueIndex = 0;
		for (uint8_t i = 0; i < mDmxNumReceivers; i++) {
			auto *receiver = &mDmxReceivers[i];
			for (uint16_t j = receiver->numBytes - 1; j >= 0; j--) {
				receiver->ptr[j] = mDmxValues[dmxValueIndex++];
			}
		}

		if (mFeadBufferReady) {
			if (mFeadPacket.isValid(mAddress) && mRequestHandler) {
				uint8_t param = mFeadPacket.bits.param;
				auto request = Request<vocab_t>(param, mFeadPacket.bits.payload);
				
				switch (mFeadPacket.bits.command) {
				case Command::GET:
					reply(mRequestHandler->get(request));
					break;
				case Command::SET:
					if (mRequestHandler->set(request)) {
						ack(Response<vocab_t>(param));
					}
					break;
				}
			}

			mFeadBufferReady = false;
		}
	}

public:
	void receive(uint8_t status, uint8_t data) override {

		if (status & (1<<FE0)) {			
			mByteCounter = 0;
			mPacketType = FEAD_PACKET_TYPE_NONE;
		}
		else {
			if (mByteCounter == 0) {
				mPacketType = data;
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
	RequestHandler *mRequestHandler;
};

using DmxSlave = Slave<uint8_t>; // is this good?

}
