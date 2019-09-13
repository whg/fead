#pragma once

#include <stdint.h>

#include "fead/hardware.hpp"
#include "fead/request.hpp"
#include "fead/response.hpp"
#include "fead/packet.hpp"

#include "fead/debug.hpp"

namespace fead {

template <typename vocab_t>
class Master : public SerialUnit {
public:
	class ReplyHandler {
	public:
		virtual void received(const Response<vocab_t> &res) = 0;
		virtual void acked() = 0;
	};

public:
	Master():
		mFeadBufferReady(false),
		mReplyHandler(nullptr)
	{}
			
	virtual ~Master() {}
	
	void get(uint8_t unit, const Request<vocab_t> &request) {
		send(Packet::create(Command::GET, FEAD_MASTER_ADDRESS, unit, request));
	}

	void set(uint8_t unit, const Request<vocab_t> &request) {
		send(Packet::create(Command::SET, FEAD_MASTER_ADDRESS, unit, request));
	}

	void setHandler(ReplyHandler* const handler) {
		mReplyHandler = handler;
	}

	void update() {
		if (mFeadBufferReady) {
			
			if (mFeadPacket.isValid(FEAD_MASTER_ADDRESS)) {
				switch (mFeadPacket.bits.command) {
				case Command::REPLY:
					if (mReplyHandler) {
						uint8_t param = mFeadPacket.bits.param;
						auto response = Response<vocab_t>(param, mFeadPacket.bits.payload);
						mReplyHandler->received(response);
					}
					break;
				case Command::ACK:
					if (mReplyHandler) {
						mReplyHandler->acked();
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
		}
		else {
			if (mByteCounter == 0) {
				mPacketType = data;
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
	volatile Packet mFeadPacket;
	volatile bool mFeadBufferReady;
	
	volatile uint8_t mByteCounter;
	volatile packet_type_t mPacketType;

protected:
	ReplyHandler *mReplyHandler;
};
	
}
