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
			uint8_t mixval = read_input(mix->srcRaw);
			mixval = apply_curve(mixval, mix->curve);
			mixval = ((int16_t)mixval * ((int16_t)mix->weight+1)) >> 8;
			mixval += mix->offset;
			
			switch ((uint8_t)mix->mltpx)
			{
				case ADD:
					mixOuts[mix->destCh] += mixval;
					break;
				case MULTIPLY:
					mixOuts[mix->destCh] *= mixval;
					break;
				case REPLACE:
					mixOuts[mix->destCh]  = mixval;
					break;
				default:
				case UNUSED:
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

