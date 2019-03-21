#pragma once

#include <stdint.h>
#include <string.h>

#define FEAD_MESSAGE_PAYLOAD_LENGTH 4

namespace fead {
	
template<typename vocab_t>
class Message {
public:
	Message(vocab_t param): mParam(param) { setValue(0); }
	Message(vocab_t param, int v): mParam(param) { setValue(v); }
	Message(vocab_t param, float v): mParam(param) { setValue(v); }
	Message(vocab_t param, uint32_t v): mParam(param) { setValue(v); }
	Message(vocab_t param, int32_t v): mParam(param) { setValue(v); }

	Message(uint8_t param, volatile uint8_t buffer[FEAD_MESSAGE_PAYLOAD_LENGTH]):
		mParam(static_cast<vocab_t>(param))
	{
		memcpy(mData.buffer, buffer, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}
	
	virtual ~Message() {}

	template<typename T>
	void setValue(const T &v) {
		memset(mData.buffer, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
		const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&v);
		memcpy(mData.buffer, ptr, sizeof(T));
	}

	vocab_t getParam() { return mParam; }
	const uint8_t* getPayloadBuffer() const { return mData.buffer; }

	void setParam(uint8_t p) { mParam = static_cast<vocab_t>(p); }
	void setPayloadBuffer(const uint8_t *d) {
		memset(mData.buffer, d, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}
	
protected:
	vocab_t mParam;
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
	
}
