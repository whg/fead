#pragma once

#include <string.h>
#include <stdlib.h>

#include "fead/master.hpp"
#include "fead/hardware.hpp"
#include "fead/auto-params.hpp"

#define FEAD_CONDUCTOR_DEFAULT_BAUD 38400ul
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
		mRxBufferReady(false),
		mRxByteCounter(0)
	{
		resetBuffer();
	}

	void init(uint8_t masterSerialUnit, uint16_t baud=FEAD_CONDUCTOR_DEFAULT_BAUD) {
		mMaster.open(masterSerialUnit);
		mMaster.setHandler(this);
		
		// conductor always is on USB port
		Debug.begin(baud);
		UCSR0B |= (1<<RXEN0) | (1<<RXCIE0);
		SerialUnit::sUnits[0] = this;
	}

	void received(const Response<vocab_t> &res, uint8_t sender) {
		Debug.print('r');
		Debug.print(sender);
		Debug.print(FEAD_CONDUCTOR_SEPARATOR);
		Debug.print(static_cast<int>(res.getParam()));
		Debug.print(FEAD_CONDUCTOR_SEPARATOR);
		if (mExpectingType == Type::FLOAT32) {
			Debug.print(res.asFloat32());
		} else if (mExpectingType == Type::INT32) {
			Debug.print(res.asInt32());
		}
		Debug.print(FEAD_CONDUCTOR_TERMINATOR);
	}

	void acked(const Response<vocab_t> &res, uint8_t sender) {
		received(res, sender);
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
			vocab_t param;
			if (*nextStart == 'i') {
				param = static_cast<vocab_t>(FEAD_SLAVE_UID_PARAM);
			}
			else if (*nextStart == 'a') {
				param = static_cast<vocab_t>(FEAD_SLAVE_ADDR_PARAM);
			}
			else {
				param = static_cast<vocab_t>(atoi(nextStart));
			}
			
			p = strchr(nextStart, FEAD_CONDUCTOR_SEPARATOR);

			if (p != NULL) {
				*p = '\0';
				nextStart = p + 1;
			} else {
				nextStart = NULL;
			}

			// uppercase characters are for floats
			mExpectingType = command < 'a' ? Type::FLOAT32 : Type::INT32;
			
			if (FEAD_CONDUCTOR_IS_GET(command)) {
				if (nextStart != NULL) {
					int value = atoi(nextStart);
					mMaster.get(number, Request<vocab_t>(param, value));
				} else {
					mMaster.get(number, Request<vocab_t>(param));
				}
				
			} else if (FEAD_CONDUCTOR_IS_SET(command)) {
				// TODO: set float too

				int value = atoi(nextStart);
				bool extraValue = false;			

				p = strchr(nextStart, FEAD_CONDUCTOR_SEPARATOR);

				if (p != NULL) {
					auto extraValue = atoi(p + 1);
					mMaster.set(number, Request<vocab_t>(param, value, extraValue));
				} else {
					mMaster.set(number, Request<vocab_t>(param, value));
				}
			}
			
			resetBuffer();
		}
	}

	void receive(uint8_t status, uint8_t data) override {
		data &= 127; // not sure why the MSB is set!
		if (!mRxBufferReady) {
			if (data == FEAD_CONDUCTOR_TERMINATOR) {
				mRxBuffer[mRxByteCounter++] = '\0';
				mRxBufferReady = true;
			} else {
				// we can overflow...
				mRxBuffer[mRxByteCounter++] = data;
			}
		}
	}

	void resetBuffer() {
		mRxByteCounter = 0;
		mRxBufferReady = false;
		memset(mRxBuffer, 0, FEAD_CONDUCTOR_RX_BUFFER_SIZE);
	}
	
protected:
	Master<vocab_t> mMaster;

    volatile bool mRxBufferReady;
	uint8_t mRxBuffer[FEAD_CONDUCTOR_RX_BUFFER_SIZE];
	volatile uint8_t mRxByteCounter;

	Type mExpectingType;
};
	
}
