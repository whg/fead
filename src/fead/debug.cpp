#include "fead/debug.hpp"

#include <avr/io.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

void _Debug::begin(uint32_t baud) {
    UBRR0L = static_cast<uint8_t>(roundf(F_CPU / (16.f * baud)) - 1);
    UCSR0C = (1<<UCSZ01) | (1<<UCSZ00); // 8 bit
    UCSR0B = (1<<TXEN0);
}

void _Debug::print(char c) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = c;
}

void _Debug::print(const char *str) {
    print(str, strlen(str));
}

void _Debug::print(const char *buffer, uint16_t len) {
    while (len--) {
		print(*buffer++);
    }
}

void _Debug::print(int32_t n) {
    char buf[12]; // 2**32 == 10 digits
    char *str = &buf[11];
    *str = '\0';
	uint32_t un = labs(n);
	
    do {
		*--str = (un % 10) + '0';
		un /= 10;
    } while(un);

	if (n < 0) {
		*--str = '-';
	}
  
    print(str);
}

void _Debug::print(int n) {
	print(static_cast<int32_t>(n));
}

void _Debug::print(uint8_t n) {
	print(static_cast<int32_t>(n));
}

void _Debug::print(float f) {
    if (isnan(f)) {
		print("nan");
    } else if (isinf(f)) {
		print("inf");
    } else {
		if (f < 0) {
			print('-');
			f = -f;
		}
		float rounding = 0.5f;
		uint8_t digits = 5;
		for (uint8_t i = 0; i < digits; i++) {
			rounding /= 10.f;
		}
		f += rounding;
  
		int integer = static_cast<int>(f);
		print(integer);
      
		float remainder = f - static_cast<float>(integer);
		print('.');
		while (digits-- > 0) {
			remainder *= 10;
			int digit = static_cast<int>(remainder);
			print(digit);
			remainder -= digit;
		}
    }
}


_Debug Debug;
