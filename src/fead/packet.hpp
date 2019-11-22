#pragma once

#include <string.h>

#include "fead/message.hpp"

#define FEAD_PACKET_TYPE_NONE 0x11
#define FEAD_PACKET_TYPE_DMX  0x00
#define FEAD_PACKET_TYPE_FEAD 0xfe

#define FEAD_PACKET_HEADER 0xfe
#define FEAD_PACKET_FOOTER 0xad

#define FEAD_PACKET_LENGTH 11

#define FEAD_BROADCAST_ADDRESS 0
#define FEAD_MASTER_ADDRESS 255

#define FEAD_COMMAND_PAYLOAD_NUM_SHIFT 5
#define FEAD_COMMAND_ARG_TYPE_SHIFT 2

namespace fead {

using packet_type_t = uint8_t;
	
enum Command { SET, GET, REPLY }; // don't go over 4

inline uint8_t get_checksum(const uint8_t *buffer) {
	uint8_t cs = 0;
	cs += buffer[0] + buffer[1] + buffer[2] + buffer[3];
	return cs;
}
	
union Packet {
	Packet() {}

    template <typename T>
    static Packet create(Command c, uint8_t sender, uint8_t destination, const MessageT<T> &msg) {
		Packet output;
		output.bits.header = FEAD_PACKET_HEADER;
		output.bits.command = static_cast<uint8_t>(c);
		output.bits.command |= (msg.getNumArgs() << FEAD_COMMAND_PAYLOAD_NUM_SHIFT);
		output.bits.command |= (static_cast<uint8_t>(msg.getArgType()) << FEAD_COMMAND_ARG_TYPE_SHIFT);
		output.bits.senderAddress = sender;
		output.bits.destinationAddress = destination;
		output.bits.param = static_cast<uint8_t>(msg.getParam());
		memcpy(output.bits.payload, msg.getPayloadBuffer(), FEAD_MESSAGE_PAYLOAD_LENGTH);
		output.bits.checksum = get_checksum(&output.bits.payload[0]);
		output.bits.footer = FEAD_PACKET_FOOTER;
		return output;
	}

	bool isValid(uint8_t address) const volatile {
		if (bits.destinationAddress != FEAD_BROADCAST_ADDRESS && address != bits.destinationAddress) {
			return false;
		}
		uint8_t v = 0;
		for (uint8_t i = 0; i < FEAD_MESSAGE_PAYLOAD_LENGTH; i++) {
			v += bits.payload[i];
		}
		return v == bits.checksum;
	}

	bool isBroadcast() const volatile { return bits.destinationAddress == FEAD_BROADCAST_ADDRESS; }

	Command getCommand() const volatile { return static_cast<Command>(bits.command & 3); }
	uint8_t getNumArgs() const volatile { return (bits.command >> FEAD_COMMAND_PAYLOAD_NUM_SHIFT) & 7; }
	ArgType getArgType() const volatile {
		return static_cast<ArgType>((bits.command >> FEAD_COMMAND_ARG_TYPE_SHIFT) & 7);
	}
	
	uint8_t buffer[FEAD_PACKET_LENGTH];
	struct {
		uint8_t header;
		uint8_t command;
		uint8_t senderAddress, destinationAddress;
		uint8_t param;
		uint8_t payload[FEAD_MESSAGE_PAYLOAD_LENGTH];
		uint8_t checksum;
		uint8_t footer;
	} bits;
};

	
}
