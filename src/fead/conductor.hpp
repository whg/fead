#pragma once

#include <string.h>
#include <stdlib.h>

#include "fead/master.hpp"
#include "fead/hardware.hpp"
#include "fead/slave.hpp"

#define FEAD_CONDUCTOR_DEFAULT_BAUD 1000000ul
#define FEAD_CONDUCTOR_RX_BUFFER_SIZE 64

#ifndef FEAD_CONDUCTOR_REQUEST_QUEUE_LENGTH
#define FEAD_CONDUCTOR_REQUEST_QUEUE_LENGTH 120
#endif

#define FEAD_CONDUCTOR_SEPARATOR ':'
#define FEAD_CONDUCTOR_TERMINATOR '\n'
#define FEAD_CONDUCTOR_GET_MASK 'G'
#define FEAD_CONDUCTOR_SET_MASK 'S'
#define FEAD_CONDUCTOR_POWER_CHAR 'p'

#define TWO_TO_THE_15 32768

#define FEAD_CONDUCTOR_IS_GET(command) \
	((command & FEAD_CONDUCTOR_GET_MASK) == FEAD_CONDUCTOR_GET_MASK)
#define FEAD_CONDUCTOR_IS_SET(command) \
	((command & FEAD_CONDUCTOR_SET_MASK) == FEAD_CONDUCTOR_SET_MASK)

namespace fead {

template <typename vocab_t>
class ConductorT : public SerialUnit, public MasterT<vocab_t>::ReplyHandler {
public:
	ConductorT():
		mRxBufferReady(false),
		mRxByteCounter(0),
		mLastSent(0), mLastReceived(0)
	{
          resetBuffer();
	}

	void init(uint8_t masterSerialUnit, uint32_t baud=FEAD_CONDUCTOR_DEFAULT_BAUD) {
		mMaster.open(masterSerialUnit);
		mMaster.setHandler(this);

		// conductor always is on USB port
		Debug.begin(baud);
		UCSR0B |= (1<<RXEN0) | (1<<RXCIE0);
		SerialUnit::sUnits[0] = this;

		DDRB |= (1<< PB0);
	}

	void setDePin(uint8_t pin) override {
		mMaster.setDePin(pin);
	}

	void received(const ResponseT<vocab_t> &res, uint8_t sender) override {
		Debug.print('r');
		Debug.print(sender);
		Debug.print(FEAD_CONDUCTOR_SEPARATOR);
		Debug.print(static_cast<int>(res.getParam()));
		Debug.print(FEAD_CONDUCTOR_SEPARATOR);
		if (res.getNumArgs() == 1) {
			switch (res.getArgType()) {
			case ArgType::FLOAT:
				Debug.print(res.asFloat());
				break;
			case ArgType::INT32:
				Debug.print(res.asInt32());
				break;
			case ArgType::INT16:
				Debug.print(res.asInt16());
				break;
			case ArgType::BOOL:
				Debug.print(res.asBool());
				break;
			case ArgType::UINT8:
			case ArgType::UINT32:
				Debug.print(int(res.asUint32()));
				break;
			}
		} else if (res.getNumArgs() == 2) {
			switch (res.getArgType()) {
			case ArgType::INT16:
				Debug.print(res.asInt16(0));
				Debug.print(FEAD_CONDUCTOR_SEPARATOR);
				Debug.print(res.asInt16(1));
				break;
			}
		}

		Debug.print(FEAD_CONDUCTOR_TERMINATOR);
		mLastReceived = millis();
	}

	void update() {
		mMaster.update();

		if (mRxBufferReady) {
			char command = mRxBuffer[0];
			char *p;

			p = strchr(&mRxBuffer[0], FEAD_CONDUCTOR_SEPARATOR);
			*p = '\0';

			int number = atoi(&mRxBuffer[1]);

			if (command == FEAD_CONDUCTOR_POWER_CHAR) {
				if (number) {
					PORTB |= (1 << PB0);
				} else {
					PORTB &= ~(1 << PB0);
				}
			}
			else {
				char *nextStart = p + 1;
				vocab_t param;
				if (*nextStart == 'i') {
					param = static_cast<vocab_t>(Slave::Param::UID);
				}
				else if (*nextStart == 'a') {
					param = static_cast<vocab_t>(Slave::Param::ADDRESS);
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

				if (FEAD_CONDUCTOR_IS_GET(command)) {
					if (nextStart != NULL) {
						auto value = atoi(nextStart);
						mMaster.get(number, RequestT<vocab_t>(param, value));
					} else {
						mMaster.get(number, RequestT<vocab_t>(param));
					}
					mLastSent = millis();
				} else if (FEAD_CONDUCTOR_IS_SET(command)) {
					auto longValue = atol(nextStart);
					int intValue = static_cast<int>(longValue);

					p = strchr(nextStart, FEAD_CONDUCTOR_SEPARATOR);

					if (p != NULL) {
						nextStart = p + 1;
						auto extraIntValue = atoi(nextStart);
						mMaster.set(number, RequestT<vocab_t>(param, intValue, extraIntValue));
					} else {
						if (labs(longValue) > TWO_TO_THE_15) {
							mMaster.set(number, RequestT<vocab_t>(param, longValue));
						} else if(strchr(nextStart, '.') != NULL) {
							float floatValue = atof(nextStart);
							mMaster.set(number, RequestT<vocab_t>(param, floatValue));
						} else {
							mMaster.set(number, RequestT<vocab_t>(param, intValue));
						}
					}
					mLastSent = millis();
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

	uint32_t getLastSent() const { return mLastSent; }
	uint32_t getLastReceived() const { return mLastReceived; }

protected:
	void resetBuffer() {
		mRxByteCounter = 0;
		mRxBufferReady = false;
		memset(mRxBuffer, 0, FEAD_CONDUCTOR_RX_BUFFER_SIZE);
	}

protected:
	MasterT<vocab_t> mMaster;

    volatile bool mRxBufferReady;
	uint8_t mRxBuffer[FEAD_CONDUCTOR_RX_BUFFER_SIZE];
	volatile uint8_t mRxByteCounter;

protected:
	uint32_t mLastSent, mLastReceived;
};

using Conductor = ConductorT<uint8_t>;

}
