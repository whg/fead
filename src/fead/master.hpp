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
		virtual void received(const Response<vocab_t> &res, uint8_t sender) = 0;
		virtual void acked(const Response<vocab_t> &res, uint8_t sender) = 0;
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
			auto *packet = const_cast<Packet*>(&mFeadPacket);
			
			if (packet->isValid(FEAD_MASTER_ADDRESS)) {
				auto response = Response<vocab_t>(packet->bits.param,
												  packet->bits.payload,
												  packet->getNumArgs(),
												  packet->getArgType());
				
				switch (packet->getCommand()) {
				case Command::REPLY:
					if (mReplyHandler) {
						mReplyHandler->received(response, packet->bits.sender_address);
					}
					break;
				case Command::ACK:
					if (mReplyHandler) {
						mReplyHandler->acked(response, packet->bits.sender_address);
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
