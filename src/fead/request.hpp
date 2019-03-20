#pragma once

#include "fead/message.hpp"

namespace fead {

template<typename vocab_t>
struct Request : public Message<vocab_t> {
	Request(vocab_t name): Message<vocab_t>(name) {}
	Request(vocab_t name, int v): Message<vocab_t>(name, v) {}
	Request(vocab_t name, float v): Message<vocab_t>(name, v) {}
	Request(vocab_t name, uint32_t v): Message<vocab_t>(name, v) {}
	Request(vocab_t name, int32_t v): Message<vocab_t>(name, v) {}
};

}
