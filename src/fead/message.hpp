#pragma once

#include <stdint.h>
#include <string.h>

#define FEAD_MESSAGE_PAYLOAD_LENGTH 4

namespace fead {

template<typename param_t>
class Message {
public:
	Message(param_t name): mName(name) {}
	Message(param_t name, int v): mName(name) { setValue(v); }
	Message(param_t name, float v): mName(name) { setValue(v); }
	Message(param_t name, uint32_t v): mName(name) { setValue(v); }
	Message(param_t name, int32_t v): mName(name) { setValue(v); }

	template<typename T>
	void setValue(const T &v) {
		memset(mData.buffer, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
		const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&v);
		memcpy(mData.buffer, ptr, sizeof(T));
	}

	
	uint16_t getParam() { return static_cast<uint16_t>(mName); }
	const uint8_t* getPayloadBuffer() const { return mData.buffer; }
		
protected:
	union {
		uint8_t buffer[FEAD_MESSAGE_PAYLOAD_LENGTH];
		struct {
			uint8_t b0, b1, b2, b3;
		} bytes;
		int32_t i32;
		uint32_t u32;
		float f;
	} mData;

	param_t mName;
};

}
