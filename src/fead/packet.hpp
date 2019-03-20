#pragma once

#include <string.h>

#include "fead/message.hpp"

#define FEAD_PACKET_LENGTH 11

namespace fead {

enum class Command { SET = 3, GET, ACK };

union packet_t {
	packet_t() {}
	
	uint8_t buffer[FEAD_PACKET_LENGTH];
	struct {
		uint8_t header;
		uint8_t command;
		uint16_t address;
		uint8_t param;
		uint8_t payload[FEAD_MESSAGE_PAYLOAD_LENGTH];
		uint8_t checksum;
		uint8_t footer;
	} bits;
};

template<typename T>
inline uint8_t get_checksum(const Message<T> &msg) {
	uint8_t cs = 0;
	const uint8_t *buffer = msg.getPayloadBuffer();
	cs += buffer[0] + buffer[1] + buffer[2] + buffer[3];
	return cs;
}

template<typename T>
inline packet_t make_get_packet(Command c, uint16_t address, T param) {
	packet_t output;
	output.bits.header = 0xfe;
	output.bits.command = static_cast<uint8_t>(c);
	output.bits.address = address;
	output.bits.param = static_cast<uint8_t>(param);
	memset(output.bits.payload, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
	output.bits.checksum = 0;
	output.bits.footer = 0xad;
	return output;
}
	
template<typename T>
inline packet_t make_set_packet(Command c, uint16_t address, const Message<T> &msg) {
	packet_t output = make_get_packet(c, address, msg.getParam());
	memcpy(output.bits.payload, msg.getPayloadBuffer(), FEAD_MESSAGE_PAYLOAD_LENGTH);
	output.bits.checksum = get_checksum(msg);
	return output;
}

}
