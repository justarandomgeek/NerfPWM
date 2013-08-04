/* ========================================================================== */
/*                                                                            */
/*   main.c                                                                   */
/*   (c) 2011 Thomas Deal                                                     */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include <avr/power.h>
#include "nerfpwm.h"


//TODO: adjust these as needed
FUSES = 
    {
        .low = LFUSE_DEFAULT | ~FUSE_CKDIV8,
        .high = HFUSE_DEFAULT & FUSE_EESAVE,
        .extended = EFUSE_DEFAULT,
    };




/*

tasks:

* read adc - in adc.c	
* I2C add-ons
* 	-ISR: receive packets, send packet, set flag (or callback?) on receive (or complete?)
* 	-flags:
* 	-data: 
* 		-out: malloc'd packets
* 		-in: myaddr:7|X:1

ISRs:

INT0_vect			# pin shared with BRAKE6 #
INT1_vect			# pin shared with PWM6 #
PCINT0_vect		# bits 4-7 may be usable / GPIO_B #
PCINT1_vect		# bits 0-3 usable if not used for analog in  #
PCINT2_vect		# some pins usable if not used for PWM/BRAKE #
WDT_vect			unused ? maybe use for threading ?
TIMER2_COMPA_vect	unused # timing forced for hardware PWM for PWM5 if used #
TIMER2_COMPB_vect	unused # timing forced for hardware PWM for PWM6 if used #
TIMER2_OVF_vect	unused # timing forced for hardware PWM for PWM5/6 if used #
TIMER1_CAPT_vect	unused # timing range forced by PWM for PWM3/4 if used #
TIMER1_COMPA_vect	unused # timing forced for hardware PWM for PWM3 if used #
TIMER1_COMPB_vect	unused # timing forced for hardware PWM for PWM4 if used #
TIMER1_OVF_vect	unused # timing forced for hardware PWM for PWM3/4 if used #
TIMER0_COMPA_vect   unused # timing forced for hardware PWM for PWM1 #
TIMER0_COMPB_vect	unused # timing forced for hardware PWM for PWM2 #
TIMER0_OVF_vect	unused # timing forced for hardware PWM for PWM1/2 #
SPI_STC_vect		# pins used by PWM4/5, GPIO #
USART_RX_vect		# pins used as BRAKE1/2 #
USART_URDE_vect	# pins used as BRAKE1/2 #
USART_TX_vect		# pins used as BRAKE1/2 #
ADC_vect			used for reading inputs
EE_READY_vect		unused
ANALOG_COMP_vect	# not usable, pins are used for PWM1/BRAKE5 #
TWI_vect			used for packet net of peripherals
SPM_REDY_vect		unused
*/

#include "mixer.h"
#include <avr/eeprom.h>
#include <avr/cpufunc.h>

EEData settings;
EEMEM EEData ee_settings;

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

#define PWMPIN1 PORTD, 6
#define BRAKEPIN1 PORTD, 0

int main(void)
{
#if F_CPU == 8000000UL
	// in case CLK_DIV_8 is programmed
	clock_prescale_set(clock_div_1);
#elif F_CPU == 1000000UL
	clock_prescale_set(clock_div_8);
#else
#error Incorrect clock speed
#endif
	
	
	
	//init_twi();
	
	adc_init();
	
	// set up PWM1/2 on Timer0 for PWM/BRAKE.
	//TODO: control this with a setting, split it out somewhere.
	//TODO: maybe make PWM1 "sing" at startup? Probably have to use CTC mode for a few rounds to do that...
	TCCR0A = _BV(COM0A1)|_BV(COM0B1)|_BV(WGM00); // Phase-correct PWM, A/B outputs set when match while down-counting, clear when up-counting
	TCCR0B = _BV(CS00); // clock with CPU clock.
	OCR0A = 0;
	OCR0B = 0;
	
	
	sei();
		
	
	
	
	while(1)
	{
		//TODO: main loop here.
		
		for(int i=0;i<MAX_MIXERS;i++)
 		{
 			apply_mix(&settings.mixData[i]);
 		}
		
		asm volatile (".global setpwms\nsetpwms:nop"); // usefull for finding this in .lss to look at...
		// copy output values from mixOuts to appropriate places...
		SET_PWM_OUT(mixOuts[0], PORTD, 0, PORTD, 6, OCR0A, TCCR0A, _BV(COM0A1));
		SET_PWM_OUT(mixOuts[1], PORTD, 1, PORTD, 5, OCR0B, TCCR0A, _BV(COM0B1));
	}
}





/*	inputs: (0-FF)
 *		0x0#
 *			0-5	ADC 0-5 
  *			6	ADC temp?
 *			7	ADC battery reading (using the 1.1 Vbg channel)?
 *			8	% time idle (or busy?)? 
 *			9-F	unassigned
 *		0x1#-0xB#	unassigned
 *		0xC#-0xF#	outputs from previous cycle, lower 6 bits = index
 */ 
uint8_t read_input(enum source inputid)
{
	switch (inputid)
	{
		case ADC0:
		case ADC1:
		case ADC2:
		case ADC3:
		case ADC4:
		case ADC5:
			return adc_val[inputid];
		
		case TIME_BUSY:
		     return 0xFF; // TODO: find some way to measure how long it takes to calculate mixes
		//....
	
		default:
			if(inputid >= 0xC0) return mixOuts[inputid & 0x3F]; 
			return 0;
	
	}
}


/*	switches: (0-7F)
 *		0x0#
 *			0	TRUE
 *			1	FALSE
 *			2-3	[reserved]  
 *			4-8	PORTB pins
 *			9-C	PWM34 pins if used as digital inputs
 *			D-F	PWM56 pins if used as digital inputs 
 *			
 *		0x1#		logic functions
 */ 
uint8_t read_logic(int8_t logicid)
{
	// negative indexes invert logic output
	if(logicid < 0) return !read_logic(-logicid);
	
	switch (logicid)
	{
		case 0x00:	// TRUE
			return 0xFF;
		case 0x01:		// FALSE
			return 0;
			
		case 0x04:	// Port B pins broken out to special header
		case 0x05:
		case 0x06:
		case 0x07:
			return PINB & _BV(logicid);
			
		case 0x08:	// PWM3
			return PINB & _BV(1);
		case 0x09:	// BRAKE3
			return PINB & _BV(0);
		case 0x0A:	// PWM4
			return PINB & _BV(2);
		case 0x0B:	// BRAKE4
			return PIND & _BV(4);
		
		case 0x0C:	// PWM5
			return PINB & _BV(3);
		case 0x0D:	// BRAKE5
			return PIND & _BV(7);
		case 0x0E:	// PWM6
			return PIND & _BV(3);
		case 0x0F:	// BRAKE6
			return PIND & _BV(2);
		
	
		case 0x10:	// Logic functions
		case 0x11:
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1A:
		case 0x1B:
		case 0x1C:
		case 0x1D:
		case 0x1E:
		case 0x1F:
			return read_logic_function(logicid);	
		
		case 0x20:	// Analog pins as digital
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
			return PINC & _BV(logicid & 0x7);
		//....
	
		default: return 0;
	
	}

}
