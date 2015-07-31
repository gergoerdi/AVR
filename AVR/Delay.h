#ifndef DELAY_H
#define DELAY_H

#include <util/delay.h>

template<int N>
struct no_operation
{
	static inline void run()
	{
		__asm__ volatile("nop");
		::no_operation<N-1>::run();
	}
};

template<>
struct no_operation<0>
{
	static inline void run()
	{
	}
};

template<int N>
static inline void nop()
{
	no_operation<N>::run();
}

static inline void delay_ms(uint16_t t)
{
	_delay_ms(t);
}

static inline void delay_us(uint16_t t)
{
	_delay_us(t);
}

#endif DELAY_H

