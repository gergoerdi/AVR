#include <avr/io.h>
#include "../AVR/Pins.h"

void delay_loop_2(uint16_t __count)
{
	__asm__ volatile
	(
		"1: sbiw %0,1" "\n\t"
		"brne 1b"
			: "=w" (__count)
			: "0" (__count)
	);
}

void delay(uint16_t n)
{
	while (n-- > 0)
		delay_loop_2(4000);
}

typedef pin_t<PD,0> led;

void setup()
{
	digital_out<led>();
}

void loop()
{
	delay(250);
	toggle<led>();
}

int main()
{
	setup();
	for (;;)
		loop();
}
