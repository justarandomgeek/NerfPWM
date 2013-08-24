#include <avr/cpufunc.h>
#include "nerfpwm.h"
#include "pwm.h"


void pwm12_init(void)
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

void pwm_init()
{
	//TODO: control this with a setting, split it out somewhere.
	pwm12_init();
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


void pwm1_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
void pwm2_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}
//void pwm3_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
//void pwm4_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}
//void pwm5_write(uint8_t value){SET_PWM_OUT(value, PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));}
//void pwm6_write(uint8_t value){SET_PWM_OUT(value, PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));}

// value=0 sets brake, otherwise OCR=value-1 (set =1 to float, =2 is slowest driven speed. Max speed 0CR=0xFE)
void pwm_write(uint8_t values[6]){
	pwm1_write(values[0]);
	pwm2_write(values[1]);
// 	pwm3_write(values[2]);
// 	pwm4_write(values[3]);
// 	pwm5_write(values[4]);
// 	pwm6_write(values[5]);
}

//void PWM34_init(void);
//void PWM56_init(void);