#include <avr/io.h>
#include <stdint.h>
#include "mixer.h"
#include "adc.h"


uint8_t mixOuts[0x40];



void apply_mix(MixData *mix)
{
	if(mix->mltpx != 0)
	{
		if(read_digital(mix->logic))
		{
			uint8_t mixval = read_analog(mix->srcRaw);
			mixval = apply_curve(mixval, mix->curve);
			mixval = ((int16_t)mixval * ((int16_t)mix->weight+1)) >> 8;
			mixval += mix->offset;
			
			switch ((uint8_t)mix->mltpx)
			{
				case MP_ADD:
					mixOuts[mix->destCh] += mixval;
					break;
				case MP_MULTIPLY:
					mixOuts[mix->destCh] *= mixval;
					break;
				case MP_REPLACE:
					mixOuts[mix->destCh]  = mixval;
					break;
				default:
				case MP_UNUSED:
					break;
				
			
			}
		
		
		}
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
uint8_t read_analog(enum analog_in inputid)
{
	switch (inputid)
	{
		case IN_ADC0 ... IN_ADC5:
		case IN_ADCTEMP:
			return adc_val[inputid];
		
		case IN_TIME_BUSY:
		     return 0xFF; // TODO: find some way to measure how long it takes to calculate mixes
		//....
		case IN_CONSTANT0:
			return 0; // adding offest will generate unsigned value 
	
		case IN_MIXOUT(0) ... IN_MIXOUT(0x3F):
			return mixOuts[inputid - IN_MIXOUT(0)];
			
		default: 
			return 0;
	
	}
}

