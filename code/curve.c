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
__attribute__((noinline)) static uint8_t interpolate_5pt(uint8_t input, uint8_t range, const uint8_t curve[])
{
	uint16_t x;
	uint8_t y;
	uint8_t first = curve[range];
	uint8_t last = curve[range+1];
	x = input * (uint8_t)(last - first);
	//y = x/64;
	y = (x<<2)>>8;
	y += first;
	return y;
}

__attribute__((noinline)) static uint8_t interpolate_9pt(uint8_t input, uint8_t range, const uint8_t curve[])
{
	uint16_t x;
	uint8_t y;
	uint8_t first = curve[range];
	uint8_t last = curve[range+1];
	x = input * (uint8_t)(last - first);
	//y = x/32;
	y = (x<<3)>>8;
	y += first;
	return y;
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
	if(curveid == -0x80)
	{
		// -0x80 is special, there is no +0x80. use 0 instead, and invert input.
		input = UINT8_MAX - input;
		curveid = 0;
	}
	// for negative curve-id values, use reversed input, and run as if positive curveid.
	if(curveid < 0)
	{
		input = UINT8_MAX - input;
		curveid = -curveid;
	}
	
	
	switch(curveid&0x7F)
	{
		case 0x00: // 00 is NOP. -0x80 also comes here for inverted value.
			return input;
		case 0x10 ... 0x1F:
				if(curveid&0x08)
				{
					return apply_curve_5point(input,curveid & 0x07);
				} else {
					return apply_curve_9point(input,curveid & 0x07);
				}

		default:
			return input; // implemented as nop, but subject to change as new curves are defined....
	
	
	}
}