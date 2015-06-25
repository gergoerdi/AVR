#include <avr/io.h>
#include <avr/interrupt.h>
#include "../AVR/Pins.h"

template<class T> T min(const T& x, const T& y) { return x < y ? x : y; }
template<class T> T max(const T& x, const T& y) { return x > y ? x : y; }

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

typedef bool dir_t;
static const bool Left = false;
static const bool Right = true;

typedef pin_t<PD,0> DIR;
typedef pin_t<PD,1> STEP;
typedef pin_t<PD,2> btnDn;
typedef pin_t<PD,3> btnUp;

enum prescale_t { prescale_1 = 1, prescale_8 = 8, prescale_64 = 64, prescale_256 = 256, prescale_1024 = 1024 };

static void timer1_config(prescale_t s)
{
	uint8_t tccr1a = 0, tccr1b = 0;

	switch (s)
	{
		case prescale_1:	tccr1b |= (1 << CS10);					break;
		case prescale_8:	tccr1b |= (1 << CS11);					break;
		case prescale_64:	tccr1b |= (1 << CS10) | (1 << CS11);	break;
		case prescale_256:	tccr1b |= (1 << CS12);					break;
		case prescale_1024:	tccr1b |= (1 << CS12) | (1 << CS10);	break;
	}

	TCCR1A = tccr1a;
	TCCR1B = tccr1b;
}

static void timer1_enable()
{
    TIMSK1 |= (1 << TOIE1);     // enable timer overflow interrupt
}

static volatile uint16_t n_steps = 0;    // tell isr how many steps to run
static volatile uint16_t cur_step = 0;   // current isr step
static volatile uint16_t max_i = 0;        // max speed
static volatile uint8_t limit_mask = 0;
static volatile uint16_t half_time_step = 10000;	// half-step duration
static volatile bool inflight = false;
static volatile uint16_t step_i = 0;
static volatile uint16_t dt = 0;
static volatile bool accel = false;
static const uint16_t micro_steps = 0;  // micro-steps shifts

static inline uint16_t eq12(uint16_t c, uint16_t n)
{
	return c - (c << 1) / ((n << 2) + 1);
}

ISR(TIMER1_OVF_vect)
{
    if (step_i < n_steps)
    {
        set<STEP>();			// pulse!
        clear<STEP>();
		TCNT1 = 65535 - (dt << 1);
		dt = eq12(dt, ++step_i);
	}
	else
		inflight = false;
}

void speedTest(dir_t dir, uint16_t n, uint16_t c)
{
    cli();                     // disable global interrupts
    inflight = true;
    accel = true;
	n_steps = n;
	step_i = 0;
	dt = c;
    write<DIR>(dir == Left);
	timer1_config(prescale_8);
	timer1_enable();
    sei();
    while (inflight)
        delay(100);
}

void setup()
{
	digital_in<btnDn, btnUp>();
	set<btnDn, btnUp>(); 			// pull-ups
	digital_out<DIR, STEP>();

//	homePosition();
}

void loop()
{
	static bool dir = false;
	delay(100);
	speedTest(dir, 2000, 15000);
	dir = !dir;
}

int main()
{
	setup();
	for (;;)
		loop();
}

