#include <avr/io.h>
#include <stdint.h>
#include "mixer.h"

uint8_t edgePrev[MAX_EDGES];

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
			return PINB & _BV(logicid - SW_B4 + 4);
			
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
			return read_digital_function(logicid - SW_FUNC0);	
		
		case SW_ADC0 ... SW_ADC5:	// Analog pins as digital.
			return PINC & _BV(logicid - SW_ADC0);
			
		case SW_EDGE0 ... SW_EDGE7:	// Rising edges, current value = high, prev = low 
			return read_digital(settings.edgeData[logicid-SW_EDGE0]) && !edgePrev[logicid-SW_EDGE0];
			
		//....
	
		default: return 0;
	
	}

}