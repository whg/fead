#pragma once

#include <stdint.h>

#include "fead/hardware.hpp"
#include "fead/request.hpp"
#include "fead/packet.hpp"

#include "fead/debug.hpp"

namespace fead {

template<typename param_t>
class Master : public SerialUnit {
public:
	Master() = default;
			
	virtual ~Master() {}
	
	void get(uint16_t unit, const Request<param_t> &request) {
		send(Packet::create(Command::GET, unit, request));
	}

	void set(uint16_t unit, const Request<param_t> &request) {
		send(Packet::create(Command::SET, unit, request));
	}

	void update() {
		if (mFeadBufferReady) {
			if (mFeadPacket.isValid()) {

				switch (mFeadPacket.bits.command) {
				case Command::GET:
					break;
				}
			}

			mFeadBufferReady = false;
		}

	}

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
	
};
	
}
