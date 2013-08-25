#include <avr/cpufunc.h>
#include "nerfpwm.h"
#include "pwm.h"


static void pwm12_init(void)
{
	// set up PWM1/2 on Timer0 for PWM/BRAKE.
	//TODO: maybe make PWM1 "sing" at startup? Probably have to use CTC mode for a few rounds to do that...
	TCCR0A = _BV(WGM00); // Phase-correct PWM, A/B outputs set when match while down-counting, clear when up-counting
	TCCR0B = _BV(CS00); // no divider on clock
	OCR0A = 0;
	OCR0B = 0;
	
	// set up appropriate direction bits for the four pins controlled...
	DDRD |= _BV(1)|_BV(1)|_BV(5)|_BV(6);
}

static void pwm34_init(void)
{
	//  0    1   2      3      4      5      6          7  
	//4if, 4ip, 4o, 2o2if, 2o2ip, 3o1ip, 1o3ip, PWM/BRAKE
	// in 3/1, odd pin out is BRAKE4
	// in 2/2, outputs are PWM4/BRAKE4
	switch(settings.pinData.PWM34Mode)
	{
		case 0: // all four as inputs, floating
			DDRB  &= ~(_BV(0)|_BV(1)|_BV(2));
			DDRD  &= ~(_BV(4));
			PORTB &= ~(_BV(0)|_BV(1)|_BV(2));
			PORTD &= ~(_BV(4));
			break;
		case 1: // all four as inputs, with pull-up
			DDRB  &= ~(_BV(0)|_BV(1)|_BV(2));
			DDRD  &= ~(_BV(4));
			PORTB |=  (_BV(0)|_BV(1)|_BV(2));
			PORTD |=  (_BV(4));
			break;
		case 2: // all four as outputs
			DDRB  |=  (_BV(0)|_BV(1)|_BV(2));
			DDRD  |=  (_BV(4));
			PORTB &= ~(_BV(0)|_BV(1)|_BV(2));
			PORTD &= ~(_BV(4));
			break;
		case 3: // 2 outputs, 2 inputs floating
			DDRB  &= ~(_BV(0)|_BV(1));
			DDRB  |=  (_BV(2));
			DDRD  |=  (_BV(4));
			PORTB &= ~(_BV(0)|_BV(1)|_BV(2));
			PORTD &= ~(_BV(4));
			break;
		case 4: // 2 outputs, 2 inputs with pull-up
			DDRB  &= ~(_BV(0)|_BV(1));
			DDRB  |=  (_BV(2));
			DDRD  |=  (_BV(4));
			PORTB |=  (_BV(0)|_BV(1));
			PORTB &= ~(_BV(2));
			PORTD &= ~(_BV(4));
			break;
		case 5: // 3 outputs, 1 input with pull-up
			DDRB  |=  (_BV(0)|_BV(1)|_BV(2));
			DDRD  &= ~(_BV(4));
			PORTB &= ~(_BV(0)|_BV(1)|_BV(2));
			PORTD |=  (_BV(4));
			break;
		case 6: // 1 output, 3 inputs with pull-up
			DDRB  &= ~(_BV(0)|_BV(1)|_BV(2));
			DDRD  |=  (_BV(4));
			PORTB &= ~(_BV(0)|_BV(1)|_BV(2));
			PORTD |=  (_BV(4));
			break;
		case 7:
			//TODO: set up Timer1 here. Or maybe below?
			break;	
	
	}

}

static void pwm56_init(void)
{
	//TODO: set up pwm56 pins/timer here
}

void pwm_init()
{
	pwm12_init();
	pwm34_init();
	pwm56_init();
}

// if value is 0, turn off PWM pin, disable PWM in timer, and turn on brake
// else, turn off brake, enable PWM, and set OCR to value-1
#define SET_PWM_OUT(VALUE, BRAKEPORT, BRAKEPIN, PWMPORT, PWMPIN, OCR, TCCR, TCCR_FLAG) \
if(VALUE) 					\
{         					\
	BRAKEPORT &= ~_BV(BRAKEPIN);	\
	_NOP();					\
	TCCR |= (TCCR_FLAG);		\
	OCR = (VALUE)-1;  			\
} else {                           \
	TCCR &= ~(TCCR_FLAG);		\
	PWMPORT &= _BV(PWMPIN);		\
	_NOP();					\
	BRAKEPORT |= _BV(BRAKEPIN);	\
}


static void pwm1_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
static void pwm2_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}
//static void pwm3_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
//static void pwm4_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}
//static void pwm5_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
//static void pwm6_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}

// value=0 sets brake, otherwise OCR=value-1 (set =1 to float, =2 is slowest driven speed. Max speed 0CR=0xFE)
void pwm_write(uint8_t values[6]){
	pwm1_write(values[0]);
	pwm2_write(values[1]);
// 	pwm3_write(values[2]);
// 	pwm4_write(values[3]);
// 	pwm5_write(values[4]);
// 	pwm6_write(values[5]);
}