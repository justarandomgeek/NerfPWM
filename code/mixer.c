
#include <stdint.h>
#include "mixer.h"

extern EEData settings;
uint8_t mixOuts[0x40];

uint8_t apply_curve_function(uint8_t input, int8_t curveid);
uint8_t apply_curve_5point(uint8_t input, int8_t curveid);
uint8_t apply_curve_9point(uint8_t input, int8_t curveid);

void apply_mix(MixData *mix)
{
	if(mix->mltpx != 0)
	{
		if(read_logic(mix->logic))
		{
			uint8_t mixval = read_input(mix->srcRaw);
			mixval = apply_curve(mixval, mix->curve);
			mixval = ((int16_t)mixval * ((int16_t)mix->weight+1)) >> 8;
			mixval += mix->offset;
			
			switch (mix->mltpx)
			{
				case ADD:
					mixOuts[mix->destCh] += mixval;
				case MULTIPLY:
					mixOuts[mix->destCh] *= mixval;
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

/*
typedef struct t_LogicData { // Custom Switches data
	int8_t  v1;			// input 1 -  index to switch or analog (determined by func)
	int8_t  v2;			// input 2 -  const, or index to switch or analog (determined by func)
	uint8_t func;			// enum of functions. comparison of analog, logic of switch
} PACKED LogicData;


0x0?		[v1 = analog, v2 = const]
0x00		v1 > const
0x01		v1 < const
0x02		v1 == const
0x03		v1 != const
	
0x1?		[v1,v2 = analog]
0x10		v1 > v2
0x11		v1 < v2
0x12		v1 == v2
0x13		v1 != v2
		
0x2?		[v1,v2 = logic]
0x20		AND
0x21		OR
0x22		XOR
*/
uint8_t read_logic_function(int8_t logicid)
{
	switch(settings.logicData[logicid].func)
	{
		default: return 0;
		
		case 0x00:
			return read_input(settings.logicData[logicid].v1) > (uint8_t)settings.logicData[logicid].v2;
		case 0x01:
			return read_input(settings.logicData[logicid].v1) < (uint8_t)settings.logicData[logicid].v2;
		case 0x02:
			return read_input(settings.logicData[logicid].v1) == (uint8_t)settings.logicData[logicid].v2;
		case 0x03:
			return read_input(settings.logicData[logicid].v1) != (uint8_t)settings.logicData[logicid].v2;
			
		
		case 0x10:
			return read_input(settings.logicData[logicid].v1) > read_input(settings.logicData[logicid].v2);
		case 0x11:
			return read_input(settings.logicData[logicid].v1) < read_input(settings.logicData[logicid].v2);
		case 0x12:
			return read_input(settings.logicData[logicid].v1) == read_input(settings.logicData[logicid].v2);
		case 0x13:
			return read_input(settings.logicData[logicid].v1) != read_input(settings.logicData[logicid].v2);
			
		case 0x20:
			return read_logic(settings.logicData[logicid].v1) && read_logic(settings.logicData[logicid].v2);
		case 0x21:
			return read_logic(settings.logicData[logicid].v1) || read_logic(settings.logicData[logicid].v2);
		case 0x22: // XOR
			//asm volatile (".global logicxor\nlogicxor:"); // usefull for finding this in .lss to look at...
			return !(read_logic(settings.logicData[logicid].v1)) != !(read_logic(settings.logicData[logicid].v2));
	}
}


/*	curves:
 *		0x0#
 *			0	x
 *			1 	x
 *			2-F	reserved 
 *		0x1#
 *			0-7	5-pt curves
 *			8-F	9-pt curves  
 *		0x2#		reserved
 *		0x3#		reserved 
 */  
uint8_t apply_curve(uint8_t input, int8_t curveid) 
{
	// for negative curve-id values, use reversed input, and run as if positive curveid.
	if(curveid < 0)
	{
		input = UINT8_MAX - input;
		curveid = -curveid;
	}
	
	
	switch(curveid&0x70)
	{
		case 0x00:
			// negating these doesn't make a lot of sense, but for all but 0 you still can.
			return apply_curve_function(input, curveid & 0x0F);
		case 0x10:
				if(curveid&0x08)
				{
					return apply_curve_5point(input,curveid & 0x07);
				} else {
					return apply_curve_9point(input,curveid & 0x07);
				}
		//case 0x20:
		//case 0x30:
		//case 0x40:
		//case 0x50:
		//case 0x60:
		//case 0x70:
		default:
			return input; // implemented as nop, but subject to change as new curves are defined....
	
	
	}
}

uint8_t apply_curve_function(uint8_t input, int8_t curveid)
{
	// reserved for future use...
	return input;
}

#define interpolate(input,steps,first,last) (int8_t)((int16_t)(input)*((last)-(first)))/(steps) + (first)
#define interpolate_5pt(input,range,id) interpolate((input),64,settings.curves5[curveid][(range)],settings.curves5[id][(range)+1])
#define interpolate_9pt(input,range,id) interpolate((input),32,settings.curves9[curveid][(range)],settings.curves9[id][(range)+1]) 

uint8_t apply_curve_5point(uint8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	switch(input>>6)
	{
	
		default: // shouldn't be needed, as all cases are covered, but gcc doesn't realize that.
		case 0x0:
			if(input == 0x00) return settings.curves5[curveid][0];
			return interpolate_5pt(input-0x00,0,curveid);
		case 0x1:
			if(input == 0x40) return settings.curves5[curveid][1];
			return interpolate_5pt(input-0x40,1,curveid);
		case 0x2:
			if(input == 0x80) return settings.curves5[curveid][2];			
			return interpolate_5pt(input-0x80,2,curveid);
		case 0x3:
			if(input == 0xC0) return settings.curves5[curveid][3];
			if(input == 0xFF) return settings.curves5[curveid][4];
			return interpolate_5pt(input-0xC0,3,curveid);
	
	}
}
	
uint8_t apply_curve_9point(uint8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	switch(input>>5)
	{
		default: // shouldn't be needed, as all cases are covered, but gcc doesn't realize that.	
		case 0x0:
			if(input == 0x00) return settings.curves9[curveid][0];
			return interpolate_9pt(input-0x00,0,curveid);
		case 0x1:
			if(input == 0x20) return settings.curves9[curveid][1];
			return interpolate_9pt(input-0x20,1,curveid);
		case 0x2:
			if(input == 0x40) return settings.curves9[curveid][2];
			return interpolate_9pt(input-0x40,2,curveid);
		case 0x3:
			if(input == 0x60) return settings.curves9[curveid][3];
			return interpolate_9pt(input-0x60,3,curveid);
		case 0x4:
			if(input == 0x80) return settings.curves9[curveid][4];
			return interpolate_9pt(input-0x80,4,curveid);
		case 0x5:
			if(input == 0xA0) return settings.curves9[curveid][5];
			return interpolate_9pt(input-0xA0,5,curveid);
		case 0x6:
			if(input == 0xC0) return settings.curves9[curveid][6];
			return interpolate_9pt(input-0xC0,6,curveid);
		case 0x7:
			if(input == 0xE0) return settings.curves9[curveid][7];
			if(input == 0xFF) return settings.curves9[curveid][8];
			return interpolate_9pt(input-0xE0,7,curveid);
	}
}





