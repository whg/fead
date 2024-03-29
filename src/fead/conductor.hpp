#pragma once

#include <string.h>
#include <stdlib.h>

#include "fead/controller.hpp"
#include "fead/hardware.hpp"
#include "fead/client.hpp"
#include "fead/version.hpp"

#define FEAD_CONDUCTOR_SERIAL_UNIT 1
#define FEAD_CONDUCTOR_DEFAULT_BAUD 1000000ul
#define FEAD_CONDUCTOR_RX_BUFFER_SIZE 300

#ifndef FEAD_CONDUCTOR_REQUEST_QUEUE_LENGTH
#define FEAD_CONDUCTOR_REQUEST_QUEUE_LENGTH 120
#endif

#define FEAD_CONDUCTOR_SEPARATOR ':'
#define FEAD_CONDUCTOR_TERMINATOR '\n'
#define FEAD_CONDUCTOR_GET_MASK 'G'
#define FEAD_CONDUCTOR_SET_MASK 'S'
#define FEAD_CONDUCTOR_POWER_CHAR 'p'
#define FEAD_CONDUCTOR_VERSION_CHAR 'v'
#define FEAD_CONDUCTOR_FLASH_CHAR 'f'

#define FEAD_FLASH_MAX_PAYLOAD 140

#define TWO_TO_THE_15 32768

#define FEAD_CONDUCTOR_IS_GET(command) \
	((command & FEAD_CONDUCTOR_GET_MASK) == FEAD_CONDUCTOR_GET_MASK)
#define FEAD_CONDUCTOR_IS_SET(command) \
	((command & FEAD_CONDUCTOR_SET_MASK) == FEAD_CONDUCTOR_SET_MASK)

namespace fead {

template <typename vocab_t>
class ConductorT : public SerialUnit, public ControllerT<vocab_t>::ReplyHandler {
public:
	ConductorT():
		mRxBufferReady(false),
		mRxByteCounter(0),
		mLastSent(0), mLastReceived(0)
	{
          resetBuffer();
	}

	void init(uint32_t baud=FEAD_CONDUCTOR_DEFAULT_BAUD) {
		mController.open(FEAD_CONDUCTOR_SERIAL_UNIT);
		mController.setHandler(this);

		// conductor always is on USB port
		Debug.begin(baud);
		UCSR0B |= (1<<RXEN0) | (1<<RXCIE0);
		SerialUnit::sUnits[0] = this;

		DDRB |= (1<< PB0);
	}

	void setDePin(uint8_t pin) override {
		mController.setDePin(pin);
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
		mController.update();

		if (mRxBufferReady) {
			char command = mRxBuffer[0];

			if (command == FEAD_CONDUCTOR_VERSION_CHAR) {
				digitalWrite(5, LOW);
				Debug.print(FEAD_CONDUCTOR_VERSION_CHAR);
				Debug.print(FEAD_VERSION);
				Debug.print(FEAD_CONDUCTOR_TERMINATOR);
				digitalWrite(5, HIGH);
			} else if (command == FEAD_CONDUCTOR_FLASH_CHAR) {
				writeFlashPayload();
			} else {
				int number = atoi(&mRxBuffer[1]);

				if (command == FEAD_CONDUCTOR_POWER_CHAR) {
					if (number) {
						PORTB |= (1 << PB0);
					} else {
						PORTB &= ~(1 << PB0);
					}
					Debug.print(FEAD_CONDUCTOR_POWER_CHAR);
					Debug.print(number);
					Debug.print(FEAD_CONDUCTOR_TERMINATOR);
				}
				else if (mRxBuffer[1] != 'u') { // incase of undefined
					char *p = strchr(&mRxBuffer[0], FEAD_CONDUCTOR_SEPARATOR);
					*p = '\0';

					char *nextStart = p + 1;
					vocab_t param;
					if (*nextStart == 'i') {
						param = static_cast<vocab_t>(Client::Param::UID);
					}
					else if (*nextStart == 'a') {
						param = static_cast<vocab_t>(Client::Param::ADDRESS);
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
							mController.get(number, RequestT<vocab_t>(param, value));
						} else {
							mController.get(number, RequestT<vocab_t>(param));
						}
					} else if (FEAD_CONDUCTOR_IS_SET(command)) {
						auto longValue = atol(nextStart);
						int intValue = static_cast<int>(longValue);

						p = strchr(nextStart, FEAD_CONDUCTOR_SEPARATOR);

						if (p != NULL) {
							nextStart = p + 1;
							auto extraIntValue = atoi(nextStart);
							mController.set(number, RequestT<vocab_t>(param, intValue, extraIntValue));
						} else {
							if (labs(longValue) > TWO_TO_THE_15) {
								mController.set(number, RequestT<vocab_t>(param, longValue));
							} else if(strchr(nextStart, '.') != NULL) {
								float floatValue = atof(nextStart);
								mController.set(number, RequestT<vocab_t>(param, floatValue));
							} else {
								mController.set(number, RequestT<vocab_t>(param, intValue));
							}
						}
					}
					mLastSent = millis();
				}
			}

			resetBuffer();
		}
	}

	void receive(uint8_t status, uint8_t data) override {
		if (!mRxBufferReady) {
			if (data == FEAD_CONDUCTOR_TERMINATOR) {
				mRxBuffer[mRxByteCounter++] = '\0';
				mRxBufferReady = true;
			} else if (mRxByteCounter < FEAD_CONDUCTOR_RX_BUFFER_SIZE) {
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
	void writeFlashPayload() {
		char temp;
		int flashNumBytes = 0;
		for (int i = 1; i < mRxByteCounter - 1; i+= 2) {
			temp = mRxBuffer[i + 2];
			mRxBuffer[i + 2] = '\0';
			mFlashBuffer[flashNumBytes++] = strtol(&mRxBuffer[i], NULL, 16);
			mRxBuffer[i + 2] = temp;
		}

		// set uart for optiboot
		UBRR1L = 8; // 115.2k baud at 16MHz
		UCSR1C = (1<<UCSZ01) | (1<<UCSZ00); // 8 bit
		UCSR1B = (1<<TXEN0);

		mController.driverEnable();

		cli();
		for (int i = 0; i < flashNumBytes; i++) {
			loop_until_bit_is_set(UCSR1A, UDRE1);
			UDR1 = mFlashBuffer[i];
			if (i > 0 && i % 32 == 0) {
				_delay_ms(10);
			}
		}
		UCSR1A |= (1<<TXC1);
		loop_until_bit_is_set(UCSR1A, TXC1);
		sei();

		mController.driverEnable(false);

		// reset uart for normal operation
		mController.open(FEAD_CONDUCTOR_SERIAL_UNIT);

		mLastSent = millis();

		Debug.print('w');
		Debug.print(flashNumBytes);
		Debug.print(FEAD_CONDUCTOR_TERMINATOR);
	}

protected:
	ControllerT<vocab_t> mController;

    volatile bool mRxBufferReady;
	char mRxBuffer[FEAD_CONDUCTOR_RX_BUFFER_SIZE];
	volatile uint16_t mRxByteCounter;

	uint8_t mFlashBuffer[FEAD_FLASH_MAX_PAYLOAD];

protected:
	uint32_t mLastSent, mLastReceived;
};

using Conductor = ConductorT<uint8_t>;

}
