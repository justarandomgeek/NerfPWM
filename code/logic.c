#include <avr/io.h>
#include <stdint.h>
#include "mixer.h"


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
__attribute__((noinline)) static uint8_t read_digital_function(int8_t logicid)
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
			return read_digital(settings.logicData[logicid].v1) && read_digital(settings.logicData[logicid].v2);
		case 0x21:
			return read_digital(settings.logicData[logicid].v1) || read_digital(settings.logicData[logicid].v2);
		case 0x22: // XOR
			//asm volatile (".global logicxor\nlogicxor:"); // usefull for finding this in .lss to look at...
			return !(read_digital(settings.logicData[logicid].v1)) != !(read_digital(settings.logicData[logicid].v2));
	}
}

uint8_t read_digital(int8_t logicid)
{
	// negative indexes invert logic output
	if(logicid < 0) return !read_digital(-logicid);	
	
	switch (logicid)
	{
		case 0x00:	// FALSE
			return 0;
		case 0x01:		// TRUE
			return 0xFF;
			
		case 0x04 ... 0x07:	// Port B pins broken out to special header
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
		
	
		case 0x10 ... 0x1F:	// Logic functions
			return read_digital_function(logicid - 0x10);	
		
		case 0x20 ... 0x27:	// Analog pins as digital
			return PINC & _BV(logicid & 0x7);
		//....
	
		default: return 0;
	
	}

}