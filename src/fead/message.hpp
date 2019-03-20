#pragma once

#include <stdint.h>
#include <string.h>

#define FEAD_MESSAGE_PAYLOAD_LENGTH 4

namespace fead {

class MessageData {
public:
	MessageData() { setValue(0); }
	MessageData(int v) { setValue(v); }
	MessageData(float v) { setValue(v); }
	MessageData(uint32_t v) { setValue(v); }
	MessageData(int32_t v) { setValue(v); }

	virtual ~MessageData() {}
	
	template<typename T>
	void setValue(const T &v) {
		memset(mData.buffer, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
		const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&v);
		memcpy(mData.buffer, ptr, sizeof(T));
	}
	
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
	
};
	
template<typename vocab_t>
class Message : public MessageData {
public:
	Message(vocab_t name): mName(name) { setValue(0); }
	Message(vocab_t name, int v): mName(name) { setValue(v); }
	Message(vocab_t name, float v): mName(name) { setValue(v); }
	Message(vocab_t name, uint32_t v): mName(name) { setValue(v); }
	Message(vocab_t name, int32_t v): mName(name) { setValue(v); }

	virtual ~Message() {}
	
	uint8_t getParam() { return static_cast<uint8_t>(mName); }
	const uint8_t* getPayloadBuffer() const { return mData.buffer; }

	void setParam(uint8_t p) { mName = static_cast<vocab_t>(p); }
	void setPayloadBuffer(const uint8_t *d) {
		memset(mData.buffer, d, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}
	
protected:
	vocab_t mName;
};

}
