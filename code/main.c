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


//TODO: adjust these as needed these
FUSES = 
    {
        .low = LFUSE_DEFAULT | ~FUSE_CKDIV8,
        .high = HFUSE_DEFAULT & FUSE_EESAVE,
        .extended = EFUSE_DEFAULT,
    };




/*

tasks:

* read buttons
* 	-ISR(s)
* 	-event/callback when button change
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

EEData settings;
EEMEM EEData ee_settings;

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
	
	sei();
	

	while(1)
	{
		//TODO: main loop here.
		
		if(adc_new_data) // or other new-data flags...?
		{
			// TODO: calculate mixes, update PWMs
			//apply_curve(TCNT0, TCNT2);
			apply_mix(&settings.mixData[0]);
			
		
		}
		
	
	
	}
}


