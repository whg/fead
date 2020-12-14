#pragma once

#include <stdint.h>

#include "fead/hardware.hpp"
#include "fead/request.hpp"
#include "fead/response.hpp"
#include "fead/packet.hpp"

#ifdef FEAD_DEBUG
#include "fead/debug.hpp"
#endif

namespace fead {

template <typename vocab_t>
class ControllerT : public SerialUnit {
public:
	class ReplyHandler {
	public:
		virtual void received(const ResponseT<vocab_t> &res, uint8_t sender) = 0;
	protected:
		ControllerT<vocab_t> *mControllerRef = nullptr;
		friend class ControllerT<vocab_t>;
	};

public:
	ControllerT():
		mFeadBufferReady(false),
		mReplyHandler(nullptr)
	{}

	virtual ~ControllerT() {}

	void get(uint8_t unit, const RequestT<vocab_t> &request) {
#ifdef FEAD_DEBUG
		Debug.print("g:");
		Debug.print(unit);
		Debug.print(":");
		Debug.println(request.getParam());
#endif
		send(Packet::create(Command::GET, FEAD_MASTER_ADDRESS, unit, request));
	}

	void set(uint8_t unit, const RequestT<vocab_t> &request) {
#ifdef FEAD_DEBUG
		Debug.print("s:");
		Debug.print(unit);
		Debug.print(":");
		Debug.print(request.getParam());
		Debug.print(":");
		Debug.println(request.asInt());
#endif
		send(Packet::create(Command::SET, FEAD_MASTER_ADDRESS, unit, request));
	}

	void setHandler(ReplyHandler* const handler) {
		mReplyHandler = handler;
		handler->mControllerRef = this;
	}

	void update() {
		if (mFeadBufferReady) {
			if (mFeadPacket.isValid(FEAD_MASTER_ADDRESS)) {
				auto response = ResponseT<vocab_t>(mFeadPacket.bits.param,
												   mFeadPacket.bits.payload,
												   mFeadPacket.getNumArgs(),
												   mFeadPacket.getArgType());

				// grab these here and allow for more data to be received
				auto command = mFeadPacket.getCommand();
				auto sender = mFeadPacket.bits.senderAddress;
				memset(mFeadPacket.buffer, 0, FEAD_PACKET_LENGTH);

				switch (command) {
				case Command::REPLY:
					if (mReplyHandler) {
#ifdef FEAD_DEBUG
						Debug.print("r:");
						Debug.print(mFeadPacket.bits.senderAddress);
						Debug.print(":");
						Debug.print(response.getParam());
						Debug.print(":");
						Debug.println(response.asInt());
#endif
						mReplyHandler->received(response, sender);
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
				mFeadPacket.buffer[mByteCounter] = data;
				if (mByteCounter >= FEAD_PACKET_LENGTH - 1) {
					mFeadBufferReady = true;
				}
			}
			mByteCounter++;
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

using Controller = ControllerT<uint8_t>;

}
