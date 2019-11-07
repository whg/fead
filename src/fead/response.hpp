#pragma once

#include "fead/message.hpp"

namespace fead {

template<typename vocab_t>
using ResponseT = MessageT<vocab_t>;

using Response = Message;
	
// extern Response EmptyResponse;
	
}
