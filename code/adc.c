/* ========================================================================== */
/*                                                                            */
/*   Filename.c                                                               */
/*   (c) 2001 Author                                                          */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include "nerfpwm.h"
#include "adc.h"

// used internal by ADC ISR to keep track of state
static uint8_t adc_state;

// set by ISR when a channel finishes, shared to app (which may clear it)
volatile uint8_t adc_new_data;


volatile adc_t adc_val[6];

/*
* read ADC values
* 	-ISR : loop through enabled channels, set NewData when wrapping from last channel
* 	-flags: 
* 		-out: NewData:1
* 		-in: ADCnEN:6
* 	-data: 
* 		-out uint16_t ADC[6] // keep holes. Maybe one day only allocate up to highest used channel?
* 		-internal unit8_t ADC_state
* 	-event/callback when NewData set
* 	-functions:
* 		init()	set up registers, start first conversion
* 		
*/

void adc_init(void)
{
	// select AREF = 5V, left-alight result, select chan 0
	
#ifdef ADC_8BIT
	ADMUX  = _BV(REFS0)|_BV(ADLAR);
#else 
	ADMUX  = _BV(REFS0);
#endif		
	
	adc_state =  0;
		
	// no ADC auto-trigger
	ADCSRB = 0;
		
	// enable ADC, start first conv (slower than others), clk/128
	ADCSRA = _BV(ADEN)|_BV(ADSC)|_BV(ADPS2)|_BV(ADPS1)|_BV(ADPS0)|_BV(ADIE);
	
	
	
	// disable digital inputs on enabled ADC channels
	DIDR0 =  (DIDR0 & 0xf0) | ( settings.pinData.ADCDir &  settings.pinData.ADCDat);
	
	// enable outputs, if any were selected
	DDRC  =  (DDRC  & 0xf0) | ( settings.pinData.ADCDir & ~settings.pinData.ADCDat);
	
	// enable input pull-ups, if any were selected
	PORTC =  (PORTC & 0xf0) | (~settings.pinData.ADCDir &  settings.pinData.ADCDat);
}


ISR(ADC_vect)
{

#ifdef ADC_8BIT
	uint8_t aval = ADCH;
#else
	uint16_t aval = ADC;
#endif	

	// set new-data flag
	adc_new_data = _BV(adc_state);
	
	
	adc_val[adc_state++] = aval;
	if(adc_state>=6) adc_state=0;
	
	// set MUX to new channel value 
	ADMUX = (ADMUX & 0xF0) | ((adc_state<4?adc_state:adc_state+2) & 0x0F);
		
	// start conversion
	ADCSRA |= _BV(ADSC);	
	
}