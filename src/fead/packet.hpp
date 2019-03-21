#pragma once

#include <string.h>

#include "fead/message.hpp"

#define FEAD_PACKET_TYPE_NONE 0x11
#define FEAD_PACKET_TYPE_DMX  0x00
#define FEAD_PACKET_TYPE_FEAD 0xfe

#define FEAD_PACKET_HEADER 0xfe
#define FEAD_PACKET_FOOTER 0xad

#define FEAD_PACKET_LENGTH 10

#define FEAD_MASTER_ADDRESS 0

namespace fead {

using packet_type_t = uint8_t;
	
enum Command { SET = 3, GET, REPLY, ACK };

inline uint8_t get_checksum(const uint8_t *buffer) {
	uint8_t cs = 0;
	cs += buffer[0] + buffer[1] + buffer[2] + buffer[3];
	return cs;
}
	
union Packet {
	Packet() {}

    template <typename T>
    static Packet create(Command c, uint8_t address, const Message<T> &msg) {
		Packet output;
		output.bits.header = FEAD_PACKET_HEADER;
		output.bits.command = static_cast<uint8_t>(c);
		output.bits.address = address;
		output.bits.param = static_cast<uint8_t>(msg.getParam());
		memcpy(output.bits.payload, msg.getPayloadBuffer(), FEAD_MESSAGE_PAYLOAD_LENGTH);
		output.bits.checksum = get_checksum(&output.bits.payload[0]);
		output.bits.footer = FEAD_PACKET_FOOTER;
		return output;
	}

	bool isValid(uint8_t address) volatile {
		if (address != bits.address) {
			return false;
		}
		uint8_t v = 0;
		for (uint8_t i = 0; i < FEAD_MESSAGE_PAYLOAD_LENGTH; i++) {
			v += bits.payload[i];
		}
		return v == bits.checksum;
	}
	
	uint8_t buffer[FEAD_PACKET_LENGTH];
	struct {
		uint8_t header;
		uint8_t command;
		uint8_t address;
		uint8_t param;
		uint8_t payload[FEAD_MESSAGE_PAYLOAD_LENGTH];
		uint8_t checksum;
		uint8_t footer;
	} bits;
};

	
}
