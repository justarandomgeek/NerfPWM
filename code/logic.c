#include <avr/io.h>
#include <stdint.h>
#include "mixer.h"


/*	switches: (0-7F)
 *		0x0#
 *			0	FALSE
 *			1	TRUE
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

#define analog(v) read_analog(settings.logicData[logicid].v)
#define digital(v) read_digital(settings.logicData[logicid].v)
#define constant(v) (uint8_t)(settings.logicData[logicid].v)

__attribute__((noinline)) static uint8_t read_digital_function(int8_t logicid)
{
	switch(settings.logicData[logicid].func)
	{
		default: return 0;
		
		case OP_AC_GT:
			return analog(v1) > constant(v2);
		case OP_AC_LT:
			return analog(v1) < constant(v2);
		case OP_AC_EQ:
			return analog(v1) == constant(v2);
		case OP_AC_NEQ:
			return analog(v1) != constant(v2);
			
		
		case OP_AA_GT:
			return analog(v1) > analog(v2);
		case OP_AA_LT:
			return analog(v1) < analog(v2);
		case OP_AA_EQ:
			return analog(v1) == analog(v2);
		case OP_AA_NEQ:
			return analog(v1) != analog(v2);
			
		case OP_DD_AND:
			return digital(v1) && digital(v2);
		case OP_DD_OR:
			return digital(v1) || digital(v2);
		case OP_DD_XOR:
			return !(digital(v1)) != !(digital(v2));
	}
}

uint8_t read_digital(int8_t logicid)
{
	// negative indexes invert logic output
	if(logicid < 0) return !read_digital(-logicid);	
	
	switch (logicid)
	{
		case SW_FALSE:
			return 0;
		case SW_TRUE:
			return 0xFF;
			
		case SW_B4 ... SW_B7:	// Port B pins broken out to special header
			return PINB & _BV(logicid);
			
		case SW_PWM3:
			return PINB & _BV(1);
		case SW_BRAKE3:
			return PINB & _BV(0);
		case SW_PWM4:
			return PINB & _BV(2);
		case SW_BRAKE4:
			return PIND & _BV(4);
		
		case SW_PWM5:
			return PINB & _BV(3);
		case SW_BRAKE5:
			return PIND & _BV(7);
		case SW_PWM6:
			return PIND & _BV(3);
		case SW_BRAKE6:
			return PIND & _BV(2);
		
	
		case SW_FUNC0 ... SW_FUNCF:	// Logic functions
			return read_digital_function(logicid - 0x10);	
		
		case SW_ADC0 ... SW_ADC5:	// Analog pins as digital.
			return PINC & _BV(logicid & 0x7);
		//....
	
		default: return 0;
	
	}

}