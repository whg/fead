#pragma once

#include "fead/message.hpp"

namespace fead {

struct Response : public MessageData {
	Response() { setValue(0); }
	Response(int v): MessageData(v) {}
	Response(float v): MessageData(v) {}
	Response(uint32_t v): MessageData(v) {}
	Response(int32_t v): MessageData(v) {}
};

extern Response EmptyResponse;
	
}
