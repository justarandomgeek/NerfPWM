
#include <stdint.h>
#include "mixer.h"

extern EEData settings;
int8_t mixOuts[0x40];


int8_t apply_curve_function(int8_t input, int8_t curveid);
int8_t apply_curve_5point(int8_t input, int8_t curveid);
int8_t apply_curve_9point(int8_t input, int8_t curveid);
/*

typedef struct t_MixData {
	uint8_t destCh:6;		// index to output channel
	uint8_t mltpx:2;		// multiplex method 0=+ 1=* 2=replace
	uint8_t srcRaw;		// index to analog inputs
	int8_t  weight;		// output = input * weight/127. 
	int8_t  offset;		// output = input + offset
	int8_t  logic;			// index to digital input, or logic function. negative index is !index. if this condition is false, skip this mix.
	int8_t  curve;			// index to a curve function, 
} PACKED MixData;

typedef struct t_LogicData { // Custom Switches data
	int8_t  v1;			// input 1 -  index to switch or analog (determined by func)
	int8_t  v2;			// input 2 -  const, or indext to switch or analog (determined by func)
	uint8_t func;			// enum of functions. comparison of analog, logic of switch
} PACKED LogicData;

typedef struct t_EEData {
	MixData   mixData[MAX_MIXERS];
	int8_t    curves5[MAX_CURVE5][5];
	int8_t    curves9[MAX_CURVE9][9];
	LogicData   logicData[MAX_LOGIC];
} PACKED EEData;
*/




/*	inputs: (0-FF)
 *		0x0#
 *			0	null 
 *			1-6	Physical ADC inputs
 *			7	ADC temp?
 *			8	ADC battery reading (using the 1.1 Vbg channel)?
 *			9	% time idle (or busy?)? 
 *			A-F	unassigned
 *		0x1#-0xB#	unassigned
 *		0xC#-0xF#	outputs from previous cycle, lower 6 bits = index
 */ 
int8_t read_input(uint8_t inputid)
{
	switch (inputid & 0xF0)
	{
		case 0x00:
			if(inputid == 0) return 0;	// null, unused mix. 
		case 0x10:
		case 0x20:
	
		default: return 0;
	
	}
	


}












#include <avr/io.h>
uint8_t test_logic(int8_t logicid){return TCNT0;}

void apply_mix(MixData *mix)
{
	if(test_logic(mix->logic))
	{
		int8_t mixval = read_input(mix->srcRaw);
		mixval = apply_curve(mixval, mix->curve);
		mixval = ((int16_t)mixval * mix->srcRaw) >> 8;
		mixval += mix->offset;
		
		switch (mix->mltpx)
		{
			case 0:
				mixOuts[mix->destCh] += mixval;
			case 1:
				mixOuts[mix->destCh] *= mixval;
			case 2:
				mixOuts[mix->destCh]  = mixval;
				break;
			default:
				break;
			
		
		}
	
	
	}
	
}



/*	curves:
 *		0x0#
 *			0	x
 *			1 	x>0
 *			2	x<0
 *			3	|x|
 *			4-7	reseved 
 *			8	f
 *			9	f>0
 *			A	f<0
 *			B	|f|
 *			C-F	reserved 
 *		0x1#
 *			0-7	5-pt curves
 *			8-F	9-pt curves  
 *		0x2#		reserved
 *		0x3#		reserved 
 */  
int8_t apply_curve(int8_t input, int8_t curveid)
{
	// for negative curve-id values, use reversed input, and run as if positive curveid.
	if(curveid < 0)
	{
		input = -input;
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

int8_t apply_curve_function(int8_t input, int8_t curveid)
{
	switch(curveid&0x0F)
	{
		//		comparison conditionals:
		//			return input<0?(input!=0?	neg_out:	0):		pos_out;	//label
		case 0x0:		return input<0?(input!=0?	input:	0):		input;	//nop (x) (can be skipped by mixer)
		case 0x1:		return input<0?(input!=0?	0:		0):		input;	//x>0
		case 0x2:		return input<0?(input!=0?	input:	0):		0;		//x<0
		case 0x3:		return input<0?(input!=0?	-input:	0):		input;	//|x|
		//case 0x4:
		//case 0x5:
		//case 0x6:
		//case 0x7:		return input;
		
		//		full-range
		case 0x8:		return input<0?(input!=0?	INT8_MIN:	0):		INT8_MAX;	//f
		case 0x9:		return input<0?(input!=0?	0:		0):		INT8_MAX;	//f>0
		case 0xA:		return input<0?(input!=0?	INT8_MIN:	0):		0;		//f<0
		case 0xB:		return input<0?(input!=0?	INT8_MIN:	INT8_MIN):INT8_MAX;	//|f| (perhaps more accurately f!=0?)
		//case 0xC:
		//case 0xD:
		//case 0xE:
		//case 0xF:		return input;
		default:		return input;

	}
}

#define interpolate(input,steps,first,last) (int8_t)((int16_t)(input)*((last)-(first)))/(steps) + (first)
#define interpolate_5pt(input,range,id) interpolate((input),64,settings.curves5[curveid][(range)],settings.curves5[id][(range)+1])
#define interpolate_9pt(input,range,id) interpolate((input),32,settings.curves9[curveid][(range)],settings.curves9[id][(range)+1]) 

int8_t apply_curve_5point(int8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	switch(input>>6)
	{
		case -0x2:
			if(input == -0x80) return settings.curves5[curveid][0];			
			return interpolate_5pt(input+0x80,0,curveid);
		case -0x1:
			if(input == -0x40) return settings.curves5[curveid][1];
			return interpolate_5pt(input+0x40,1,curveid);
		default: // shouldn't be needed, as all cases are covered, but gcc doesn't realize that.
		case 0x0:
			if(input == 0x00) return settings.curves5[curveid][2];
			return interpolate_5pt(input-0x00,2,curveid);
		case 0x1:
			if(input == 0x40) return settings.curves5[curveid][3];
			if(input == 0x7F) return settings.curves5[curveid][4];
			return interpolate_5pt(input-0x40,3,curveid);
	}
}
	
int8_t apply_curve_9point(int8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	switch(input>>5)
	{
		case -0x4:
			if(input == -0x80) return settings.curves9[curveid][0];			
			return interpolate_9pt(input+0x80,0,curveid);
		case -0x3:
			if(input == -0x60) return settings.curves9[curveid][1];
			return interpolate_9pt(input+0x60,1,curveid);
		case -0x2:
			if(input == -0x40) return settings.curves9[curveid][2];
			return interpolate_9pt(input+0x40,2,curveid);
		case -0x1:
			if(input == -0x20) return settings.curves9[curveid][3];
			return interpolate_9pt(input+0x20,3,curveid);
		
		default: // shouldn't be needed, as all cases are covered, but gcc doesn't realize that.	
		case 0x0:
			if(input == 0x00) return settings.curves9[curveid][4];
			return interpolate_9pt(input-0x00,4,curveid);
			
		case 0x1:
			if(input == 0x20) return settings.curves9[curveid][5];
			return interpolate_9pt(input-0x20,5,curveid);
		case 0x2:
			if(input == 0x40) return settings.curves9[curveid][6];
			return interpolate_9pt(input-0x40,6,curveid);
		case 0x3:
			if(input == 0x60) return settings.curves9[curveid][7];
			if(input == 0x7F) return settings.curves9[curveid][8];
			return interpolate_9pt(input-0x60,7,curveid);
	}
}





