#pragma once

#define CREATE_FEAD_TYPES(prefix, vocab)						\
	using prefix##Master  = fead::Master<vocab>;				\
	using prefix##Slave   = fead::Slave<vocab>;					\
	using prefix##Message = fead::Message<vocab>;				\
	using prefix##Request = fead::Request<vocab>;				\
	using prefix##Response = fead::Response<vocab>;


	
