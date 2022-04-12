#pragma once

#include <stdint.h>
#include <string.h>

#define FEAD_MESSAGE_PAYLOAD_LENGTH 4
#define FEAD_MESSAGE_VALUE_NOT_SET 0xff

namespace fead {

enum class ArgType { NONE, INT16, FLOAT, UINT32, INT32, BOOL, UINT8 }; // don't go over 8

template <typename vocab_t>
class MessageT {
public:
	MessageT(): mArgType(ArgType::NONE) {}
	MessageT(vocab_t param): mParam(param), mNumArgs(0), mArgType(ArgType::NONE) { mData.uint32 = 0; }

	MessageT(vocab_t param, int v): mParam(param), mArgType(ArgType::INT16) { setValue(v); }
	MessageT(vocab_t param, float v): mParam(param), mArgType(ArgType::FLOAT) { setValue(v); }
	MessageT(vocab_t param, double v): mParam(param), mArgType(ArgType::FLOAT) { setValue(v); }
	MessageT(vocab_t param, uint32_t v): mParam(param), mArgType(ArgType::UINT32) { setValue(v); }
	MessageT(vocab_t param, int32_t v): mParam(param), mArgType(ArgType::INT32) { setValue(v); }
	MessageT(vocab_t param, bool v): mParam(param), mArgType(ArgType::BOOL) { setValue(v); }
	MessageT(vocab_t param, uint8_t v): mParam(param), mArgType(ArgType::UINT8) { setValue(v); }

	MessageT(vocab_t param, int16_t v1, int16_t v2):
		mParam(param),
		mNumArgs(2),
		mArgType(ArgType::INT16)
	{
		mData.int16s[0] = v1;
		mData.int16s[1] = v2;
	}

	MessageT(vocab_t param, uint8_t buffer[FEAD_MESSAGE_PAYLOAD_LENGTH], uint8_t numArgs, ArgType argType):
		mParam(param),
		mNumArgs(numArgs),
		mArgType(argType)
	{
		memcpy(mData.buffer, buffer, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}

	virtual ~MessageT() {}

	static MessageT<vocab_t> none() {
		return MessageT<vocab_t>(static_cast<vocab_t>(0));
	}

	template <typename T>
	void setValue(const T &v) {
		memset(mData.buffer, 0, FEAD_MESSAGE_PAYLOAD_LENGTH);
		const uint8_t *ptr = reinterpret_cast<const uint8_t*>(&v);
		memcpy(mData.buffer, ptr, sizeof(T));
		mNumArgs = 1;
	}

	vocab_t getParam() const { return mParam; }
	const uint8_t* getPayloadBuffer() const { return mData.buffer; }

	int32_t asInt32() const { return mData.int32; }
	uint32_t asUint32() const { return mData.uint32; }
	float asFloat32() const { return mData.float32; }
	float asFloat() const { return asFloat32(); }
	int asInt() const { return static_cast<int>(mData.int32); }
	bool asBool() const { return !!mData.buffer[0]; }

	int16_t asInt16(uint8_t index=0) const { return mData.int16s[index]; }

	void setParam(uint8_t p) { mParam = static_cast<vocab_t>(p); }
	void setPayloadBuffer(const uint8_t *d) {
		memset(mData.buffer, d, FEAD_MESSAGE_PAYLOAD_LENGTH);
	}

	uint8_t getNumArgs() const { return mNumArgs; }
	ArgType getArgType() const { return mArgType; }

	bool isNone() const { return mArgType == ArgType::NONE; }

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
	uint8_t mNumArgs;
	ArgType mArgType;
};

using Message = MessageT<uint8_t>;

}
