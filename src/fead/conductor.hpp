#pragma once

#include <string.h>

#include "fead/master.hpp"
#include "fead/hardware.hpp"

#define FEAD_CONDUCTOR_BAUD 38400
#define FEAD_CONDUCTOR_RX_BUFFER_SIZE 64

#define FEAD_CONDUCTOR_SEPARATOR ':'
#define FEAD_CONDUCTOR_TERMINATOR '\n'
#define FEAD_CONDUCTOR_GET_MASK 'G'
#define FEAD_CONDUCTOR_SET_MASK 'S'

#define FEAD_CONDUCTOR_IS_GET(command) \
	((command & FEAD_CONDUCTOR_GET_MASK) == FEAD_CONDUCTOR_GET_MASK)
#define FEAD_CONDUCTOR_IS_SET(command) \
	((command & FEAD_CONDUCTOR_SET_MASK) == FEAD_CONDUCTOR_SET_MASK)

namespace fead {

enum class Type { FLOAT32, INT32 };
	
template <typename vocab_t>
class Conductor : public SerialUnit, public Master<vocab_t>::ReplyHandler {
public:
	Conductor():
		mRxBufferReady(false)
	{}

	void init(uint8_t masterSerialUnit) {
		mMaster.open(masterSerialUnit);
		mMaster.setHandler(this);
		
		// conductor always is on USB port
		Debug.begin(FEAD_CONDUCTOR_BAUD);
		UCSR0B |= (1<<RXEN0) | (1<<RXCIE0);
		SerialUnit::sUnits[0] = this;
	}

	void received(const Response<vocab_t> &res) {
		if (mExpectingType == Type::FLOAT32) {
			Debug.println(res.asFloat32());
		} else if (mExpectingType == Type::INT32) {
			Debug.println(res.asInt32());
		}
	}

	void acked() {
		Debug.println('=');
	}

	void update() {
		mMaster.update();
		
		if (mRxBufferReady) {
			char command = mRxBuffer[0];
			char *p;

			p = strchr(&mRxBuffer[0], FEAD_CONDUCTOR_SEPARATOR);
			*p = '\0';
				
			int number = atoi(&mRxBuffer[1]);

			char *nextStart = p + 1;
			p = strchr(nextStart, FEAD_CONDUCTOR_SEPARATOR);
			*p = '\0';
			
			auto param = static_cast<vocab_t>(atoi(nextStart));
			nextStart = p + 1;

			// uppercase characters are for floats
			mExpectingType = command < 'a' ? Type::FLOAT32 : Type::INT32;
			
			if (FEAD_CONDUCTOR_IS_GET(command)) {
				mMaster.get(number, Request<vocab_t>(param));
			} else if (FEAD_CONDUCTOR_IS_SET(command)) {
				// TODO: set float too
				int value = atoi(nextStart);
				mMaster.set(number, Request<vocab_t>(param, value));
			}
			
			mRxByteCounter = 0;
			mRxBufferReady = false;
		}
	}

	void receive(uint8_t status, uint8_t data) override {
		data &= 127; // not sure why the MSB is set!
		if (!mRxBufferReady) {
			if (data == FEAD_CONDUCTOR_TERMINATOR) {
				mRxBuffer[mRxByteCounter++] = 0;
				mRxBufferReady = true;
			} else {
				// we can overflow...
				mRxBuffer[mRxByteCounter++] = data;
			}
		}
	}
	
protected:
	Master<vocab_t> mMaster;

    volatile bool mRxBufferReady;
	uint8_t mRxBuffer[FEAD_CONDUCTOR_RX_BUFFER_SIZE];
	volatile uint8_t mRxByteCounter;

	Type mExpectingType;
};
	
}
