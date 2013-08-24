#include <stdint.h>
#include "mixer.h"

/*	curves:
 *		0x0#
 *			0-F	reserved 
 *		0x1#
 *			0-7	5-pt curves
 *			8-F	9-pt curves  
 *		0x2#		reserved
 *		0x3#		reserved 
 */  

#define interpolate(input,steps,first,last) (int8_t)((int16_t)(input)*(uint8_t)((last)-(first)))/(steps) + (first)

static uint8_t interpolate_5pt(uint8_t input, uint8_t range, uint8_t curve[])
{
	uint16_t x;
	uint8_t first = curve[range];
	uint8_t last = curve[range+1];
	x = input * (uint8_t)(last - first);
	x /= 64;
	x += first;
	return (uint8_t)(x & 0xFF);
	//return interpolate((input),64,settings.curves5[id][(range)],settings.curves5[id][(range)+1]);
}

static uint8_t interpolate_9pt(uint8_t input, uint8_t range, uint8_t curve[])
{
	uint16_t x;
	uint8_t first = curve[range];
	uint8_t last = curve[range+1];
	x = input * (uint8_t)(last - first);
	x /= 32;
	x += first;
	return (uint8_t)(x & 0xFF);
	
	//return interpolate((input),32,settings.curves9[id][(range)],settings.curves9[id][(range)+1]);
} 

static uint8_t apply_curve_5point(uint8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	
	uint8_t *curve = settings.curves5[curveid];
	
	switch(input)
	{
		case 0x00: return curve[0];
		case 0x01 ... 0x3F:	return interpolate_5pt(input-0x00,0,curve);
		case 0x40: return curve[1];
		case 0x41 ... 0x7F:	return interpolate_5pt(input-0x40,1,curve);
		case 0x80: return curve[2];			
		case 0x81 ... 0xBF:	return interpolate_5pt(input-0x80,2,curve);
		case 0xC0: return curve[3];
		case 0xC1 ... 0xFE:	return interpolate_5pt(input-0xC0,3,curve);
		case 0xFF: return curve[4];
	}
}
	
static uint8_t apply_curve_9point(uint8_t input, int8_t curveid)
{
	curveid = curveid & 0x7;
	
	uint8_t *curve = settings.curves9[curveid];
		
	switch(input)
	{
		default: // shouldn't be needed, as all cases are covered, but gcc doesn't realize that.	
		case 0x00: return curve[0];
		case 0x01 ... 0x1F: return interpolate_9pt(input-0x00,0,curve);
		case 0x20: return curve[1];
		case 0x21 ... 0x3F: return interpolate_9pt(input-0x20,1,curve);
		case 0x40: return curve[2];
		case 0x41 ... 0x5F: return interpolate_9pt(input-0x40,2,curve);
		case 0x60: return curve[3];
		case 0x61 ... 0x7F: return interpolate_9pt(input-0x60,3,curve);
		case 0x80: return curve[4];
		case 0x81 ... 0x9F: return interpolate_9pt(input-0x80,4,curve);
		case 0xA0: return curve[5];
		case 0xA1 ... 0xBF: return interpolate_9pt(input-0xA0,5,curve);
		case 0xC0: return curve[6];
		case 0xC1 ... 0xDF: return interpolate_9pt(input-0xC0,6,curve);
		case 0xE0: return curve[7];
		case 0xE1 ... 0xFE:	return interpolate_9pt(input-0xE0,7,curve);
		case 0xFF: return curve[8];
	}
}





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