/* ========================================================================== */
/*                                                                            */
/*   main.c                                                                   */
/*   (c) 2011 Thomas Deal                                                     */
/*                                                                            */
/*   Description                                                              */
/*                                                                            */
/* ========================================================================== */

#include <avr/power.h>
#include <avr/eeprom.h>
#include "nerfpwm.h"
#include "mixer.h"
#include "adc.h"
#include "pwm.h"

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



/*
defaults: 

PWM1 =  idle (0=brake, or 50ish probably for idle-active.
PWM1 += ADC0 when [rev button]

PWM2 = brake
PWM2 = ADC1 when [trigger]
*/

#define SIMPLEMIX(dest,mlpx,src) {.destCh = (dest),.mltpx=(mlpx),.srcRaw=(src),.weight=0xff,.offset=0,.logic=SW_TRUE,.curve=0}

EEData settings;
EEMEM EEData ee_settings= {
	.mixData = {SIMPLEMIX(PWM1,REPLACE,ADC0), SIMPLEMIX(PWM1,ADD,ADC1)},
	.curves5={},
	.curves9={},
	.logicData={}
};



int main(void)
{
	// in case CLK_DIV_8 is programmed
	clock_prescale_set(clock_div_1);
	
	eeprom_read_block(&settings,&ee_settings,sizeof(settings));
	
	//init_twi();
	
	adc_init();
	pwm_init();
		
	
	sei();
		
	
	while(1){
		if(adc_new_data)
		{
			adc_new_data=0;
			
			for(int i=0;i<MAX_MIXERS;i++)
			{
 				apply_mix(&settings.mixData[i]);
 			}
		
			// copy output values from mixOuts to appropriate places...
			pwm_write(&mixOuts[PWM1]);	
			
		}
	}
}




