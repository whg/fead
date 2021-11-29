#pragma once

#include <stdint.h>
#include <avr/io.h>
#include <EEPROM.h>

#include "fead/hardware.hpp"
#include "fead/message.hpp"
#include "fead/packet.hpp"
#include "fead/response.hpp"
#include "fead/request.hpp"

#ifndef FEAD_CLIENT_MAX_DMX_RECEIVERS
#define FEAD_CLIENT_MAX_DMX_RECEIVERS 5
#endif

#ifndef FEAD_CLIENT_MAX_DMX_CHANNELS
#define FEAD_CLIENT_MAX_DMX_CHANNELS 8
#endif

#ifndef FEAD_CLIENT_RECEIVED_THRESHOLD
#define FEAD_CLIENT_RECEIVED_THRESHOLD 45000ul
#endif

#define FEAD_CLIENT_UID_AS_ADDRESS 0xff
#define FEAD_BROADCAST_REPLY_TIME_SPACE 2
#define FEAD_BROADCAST_POST_PAUSE 500

#define FEAD_NOT_RECEIVED 0

namespace fead {

struct dmx_receiver_t {
	uint16_t channel;
	uint8_t numBytes;
	uint8_t *ptr;
};

enum EEPROMSlot {
  UID = 200,
};


template <typename vocab_t>
class ClientT : public SerialUnit {
public:
	class RequestHandler {
	public:
		virtual ResponseT<vocab_t> get(const RequestT<vocab_t> &req) = 0;
		virtual bool set(const RequestT<vocab_t> &req) = 0;
	protected:
		ClientT<vocab_t> *mClientRef = nullptr;
		friend class ClientT<vocab_t>;
	};

	enum Param {
		UID = 255,
		ADDRESS = 254,
		DISCOVER = 253,
	};

public:
	ClientT(uint8_t address=FEAD_CLIENT_UID_AS_ADDRESS):
		mAddress(address),
		mDmxChannelCounter(0),
		mFeadBufferReady(false),
		mRequestHandler(nullptr),
		mLastMessageTime(FEAD_NOT_RECEIVED),
		mResponseSendTime(0),
		mResponseDelay(0)
	{}

	virtual ~ClientT() {}

	void open(uint8_t number) override {
		SerialUnit::open(number);

		EEPROM.get(EEPROMSlot::UID, mUid);
		if (mAddress == FEAD_CLIENT_UID_AS_ADDRESS) {
			mAddress = mUid;
		}
		mResponseDelay = mAddress * FEAD_BROADCAST_REPLY_TIME_SPACE;
	}

	void reply(const MessageT<vocab_t> &response) {
		SerialUnit::send(Packet::create(Command::REPLY, mAddress, FEAD_MASTER_ADDRESS, response));
	}

	void send(uint8_t address, const RequestT<vocab_t> &request) {
		// don't send something if the network is waiting for broadcast replies
		if (millis() - mBroadcastReceived > FEAD_BROADCAST_POST_PAUSE) {
			SerialUnit::send(Packet::create(Command::REPLY, mAddress, address, request));
		}
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
		mResponseDelay = mAddress * FEAD_BROADCAST_REPLY_TIME_SPACE;
	}

	void setHandler(RequestHandler* const handler) {
		mRequestHandler = handler;
		handler->mClientRef = this;
	}

	bool isReceiving() {
		return mLastMessageTime > 0 && millis() - mLastMessageTime < FEAD_CLIENT_RECEIVED_THRESHOLD;
	}

	void update() {

		uint8_t dmxValueIndex = 0;
		for (uint8_t i = 0; i < mDmxNumReceivers; i++) {
			auto *receiver = &mDmxReceivers[i];
			for (int16_t j = receiver->numBytes - 1; j >= 0; j--) {
				receiver->ptr[j] = mDmxValues[dmxValueIndex++];
			}
		}

		auto now = millis();

		if (mFeadBufferReady) {
			if (mFeadPacket.isValid(mAddress)) {
				auto param = static_cast<vocab_t>(mFeadPacket.bits.param);

				// library based getters and setters
				if (param == Param::UID && mFeadPacket.getCommand() == Command::GET) {
					auto response = ResponseT<vocab_t>(param, mUid);
					if (mFeadPacket.isBroadcast()) {
						setQueuedResponse(now, response);
					} else {
						reply(response);
					}
				}
				else if (param == Param::ADDRESS) {
					ResponseT<vocab_t> response;
					switch (mFeadPacket.getCommand()) {
					case Command::SET:
						setAddress(mFeadPacket.bits.payload[0]);
						// follow through
					case Command::GET:
						response = ResponseT<vocab_t>(param, mAddress);
						if (mFeadPacket.isBroadcast()) {
							setQueuedResponse(now, response);
						} else {
							reply(response);
						}
						break;
					}
				}
				else if (param == Param::DISCOVER) {
					if (mFeadPacket.getCommand() == Command::GET
						&& mFeadPacket.isBroadcast()
						&& mLastMessageTime == FEAD_NOT_RECEIVED) {
						ResponseT<vocab_t> response(param, mUid);
						setQueuedResponse(now, response);
					}
				}
				// user defined
				else if (mRequestHandler) {
					auto request = RequestT<vocab_t>(mFeadPacket.bits.param,
													 mFeadPacket.bits.payload,
													 mFeadPacket.getNumArgs(),
													 mFeadPacket.getArgType());

					switch (mFeadPacket.getCommand()) {
					case Command::GET:
					{
						auto response = mRequestHandler->get(request);
						if (!response.isNone()) {
							if (mFeadPacket.isBroadcast()) {
								setQueuedResponse(now, response);
							} else {
								reply(response);
							}
						}
						break;
					}
					case Command::SET:
						if (mRequestHandler->set(request)) {
							if (mFeadPacket.isBroadcast()) {
								setQueuedResponse(now, request);
							} else {
								reply(request);
							}
						}
						break;
					}
				}

				mLastMessageTime = now;
			}

			mPacketType = FEAD_PACKET_TYPE_NONE;
			mFeadBufferReady = false;
		}

		if (mResponseSendTime && now >= mResponseSendTime) {
			reply(mQueuedResponse);
			mResponseSendTime = 0;
		}
	}

	uint8_t getUid() const { return mUid; }
	uint8_t getAddress() const { return mAddress; }

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
				if (mByteCounter < FEAD_PACKET_LENGTH) {
					mFeadPacket.buffer[mByteCounter] = data;
				}
				if (mByteCounter >= FEAD_PACKET_LENGTH - 1) {
					mFeadBufferReady = true;
				}
			}
			mByteCounter++;
		}
	}

protected:
	bool isReading() const { return mByteCounter < FEAD_PACKET_LENGTH && mByteCounter > 0; }

	void setQueuedResponse(uint32_t now, const ResponseT<vocab_t> &response) {
		mBroadcastReceived = now;
		mQueuedResponse = response;
		mResponseSendTime = now + mResponseDelay;
	}

protected:
	uint8_t mUid, mAddress;
	bool mUseUidAsAddress;

	dmx_receiver_t mDmxReceivers[FEAD_CLIENT_MAX_DMX_RECEIVERS];
	uint8_t mDmxNumReceivers;

	uint16_t mDmxChannels[FEAD_CLIENT_MAX_DMX_CHANNELS];
	volatile uint8_t mDmxValues[FEAD_CLIENT_MAX_DMX_CHANNELS];
	uint8_t mDmxChannelCounter;

	volatile Packet mFeadPacket;
	volatile bool mFeadBufferReady;

	volatile uint8_t mByteCounter;
	volatile packet_type_t mPacketType;

protected:
	RequestHandler *mRequestHandler;

protected:
	uint32_t mLastMessageTime;

protected:
	ResponseT<vocab_t> mQueuedResponse;
	uint32_t mResponseSendTime;
	uint32_t mResponseDelay;
	uint32_t mBroadcastReceived;
};

using DmxClient = ClientT<uint8_t>; // is this good?
using Client = ClientT<uint8_t>;

}
