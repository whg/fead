#pragma once

#include <stdint.h>

// This is a simple replacement for Serial.print
// don't push it too hard!

class _Debug {
public:
	void begin(uint32_t baud);

	void print(char c);
	void print(const char *str);
	void print(const char *buffer, uint16_t len);
	void print(int32_t n);
	void print(int n);
	void print(uint8_t n);
	void print(float f);
	
	template<typename T>
	void println(T v) {
		print(v);
		print("\r\n");
	}
};

extern _Debug Debug;
