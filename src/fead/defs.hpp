#pragma once

#define CREATE_FEAD_TYPES(prefix, vocab)						\
	using prefix##Master  = fead::MasterT<vocab>;				\
	using prefix##Slave   = fead::SlaveT<vocab>;				\
	using prefix##Message = fead::MessageT<vocab>;				\
	using prefix##Request = fead::RequestT<vocab>;				\
	using prefix##Response = fead::ResponseT<vocab>

#define MAKE_GET_CASE(key, value) case key: return fead::Response(key, value)
#define MAKE_SET_CASE(key, value, func) case key: value = req.func(); break;
