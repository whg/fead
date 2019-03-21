#pragma once

#include "fead/message.hpp"

namespace fead {

template<typename vocab_t>
using Request = Message<vocab_t>;

}
