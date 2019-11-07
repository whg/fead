#pragma once

#include "fead/message.hpp"

namespace fead {

template<typename vocab_t>
using RequestT = MessageT<vocab_t>;

using Request = Message;

}
