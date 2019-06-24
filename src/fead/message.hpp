#pragma once

#include <stdint.h>
#include <string.h>

#define FEAD_MESSAGE_PAYLOAD_LENGTH 4
#define FEAD_MESSAGE_VALUE_NOT_SET 0xff

namespace fead {
	
template <typename vocab_t>
class Message {
public:
	Message(vocab_t param): mParam(param) { setValue(FEAD_MESSAGE_VALUE_NOT_SET); }
	Message(vocab_t param, int v): mParam(param) { setValue(v); }
	Message(vocab_t param, float v): mParam(param) { setValue(v); }
	Message(vocab_t param, uint32_t v): mParam(param) { setValue(v); }
	Message(vocab_t param, int32_t v): mParam(param) { setValue(v); }
	Message(vocab_t param, bool v): mParam(param) { setValue(v); }
	
	Message(vocab_t param, int16_t v1, int16_t v2): mParam(param) {
		mData.int16s[0] = v1;
		mData.int16s[1] = v2;
	}

	Message(uint8_t param, volatile uint8_t buffer[FEAD_MESSAGE_PAYLOAD_LENGTH]):
		mParam(static_cast<vocab_t>(param))
	{
		memcpy(mData.buffer, buffer, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}
	
	virtual ~Message() {}

	template <typename T>
	void setValue(const T &v) {
		memset(mData.buffer, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
		const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&v);
		memcpy(mData.buffer, ptr, sizeof(T));
	}

	vocab_t getParam() const { return mParam; }
	const uint8_t* getPayloadBuffer() const { return mData.buffer; }

	int32_t asInt32() const { return mData.int32; }
	float asFloat32() const { return mData.float32; }
	float asFloat() const { return asFloat32(); }
	int asInt() const { return static_cast<int>(mData.int32); }
	bool asBool() const { return !!mData.buffer[0]; }

	int16_t asInt16(uint8_t index=0) { return mData.int16s[index]; }
			
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
		int32_t int32;
		uint32_t uint32;
		float float32;
		int16_t int16s[2];
	} mData;
};
	
}
