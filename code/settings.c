#include <stdint.h>
#include <avr/eeprom.h>
#include "settings.h"

/*
defaults: 

PWM1 =  idle (0=brake, or 50ish probably for idle-active.
PWM1 += ADC0 when [rev button]

PWM2 = brake
PWM2 = ADC1 when [trigger]
*/

/*
typedef struct t_PinData {
	// ADC config for ADC 0-3
	// Load directly into low nibble DDRC/PORTC, unless both bits set, then set DIDR and monitor ADC!
	uint8_t	ADCDir:4;
	uint8_t	ADCDat:4;
	
	uint8_t	ADC6Enable:1;
	uint8_t	ADC7Enable:1;
	uint8_t	PWM34Mode:3; //4if, 4ip, 4o, 2o2if, 2o2ip, 3o1ip, 1o3ip, PWM/BRAKE
	uint8_t	PWM56Mode:3; //4if, 4ip, 4o, 2o2if, 2o2ip, 3o1ip, 1o3ip, PWM/BRAKE
	
	uint8_t	BDir:4;	// load these two directly in high nibble of DDRB/PORTB
	uint8_t	BDat:4;	

} PACKED PinData;
*/

#define SIMPLEMIX(dest,mlpx,src) {.destCh = (dest),.mltpx=(mlpx),.srcRaw=(src),.weight=0xff,.offset=0,.logic=SW_TRUE,.curve=0}
#define MIX(dest,mlpx,src,wt,ost,sw,crv) {.destCh = (dest),.mltpx=(mlpx),.srcRaw=(src),.weight=(wt),.offset=(ost),.logic=(sw),.curve=(crv)}

#define LOGIC(in1,function,in2) {.v1=(in1),.v2=(in2),.func=(function)}

#define NONE			0
#define RAPIDSTRIKE		1
#define STRYFE			2
#define RAPIDSTRIKE2	3

#ifndef PROFILE
#define PROFILE RAPIDSTRIKE
#endif

#if PROFILE == NONE
EEData settings;
static EEMEM EEData ee_settings = {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(OUT_PWM1,MP_REPLACE,IN_CONSTANT0,0xff,1,SW_TRUE,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0xff,0,SW_TRUE,0),
		},
	.curves5={},
	.curves9={},
	.logicData={},
	.pinData={
		// All ADC inputs
		.ADCDir=0x0,	.ADCDat=0x0,
		
		// endable ADC 6 and 7
		.ADC6Enable=1,	.ADC7Enable=1,
		
		// disable PWM34 and PWM56 - four HIZ pins each
		.PWM34Mode=0,	.PWM56Mode=0,
		
		// four HIZ pins on port B
		.BDir=0,		.BDat=0
	},
	.edgeData={},
};

#elif PROFILE == RAPIDSTRIKE
/*
my RS:
	ADC0 = digital pusher idle (pressed when pusher fully retracted)
	ADC1 = digital trigger
	ADC2 = digital rev
	ADC3 = unused
	ADC4 = front pot
	ADC5 = rear pot
	
	PWM1 = rev
	PWM2 = pusher

*/

//TODO: curve names?


EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(OUT_PWM1,MP_REPLACE,IN_CONSTANT0,0xff,1,SW_TRUE,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0xff,0,SW_TRUE,0),
		
		// rev half way when rev pulled but not firing
		MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0x80,0,-SW_ADC2,0),
		
		// when firing, even if rev trigger not pulled, rev to about 3/4, and run half RoF
		MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0xB0,0,SW_FUNC0,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_ADC5,0xC0,0,SW_FUNC0,0),
		
		// when both triggers pulled, rev and pusher both to full speed (set by knob)
		MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0xD0,0,SW_FUNC1,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_ADC5,0xF0,0,SW_FUNC1,0),
		
		
		},
	.curves5={},
	.curves9={},
	.logicData={
		LOGIC( SW_ADC0, OP_DD_OR, -SW_ADC1), 	// 0x21 = OR, output of this should be used to trigger pusher to activate
										// Firing: pusher not retracted or trigger pulled
		LOGIC(SW_FUNC0, OP_DD_AND, -SW_ADC2), 	// 0x20 = AND, Firing && rev pulled, required for full range/speed
										// full: firing and reved 
	},
	.pinData={
		// input and pull-ups for low four ADC pins (fourth is currently unused)
		.ADCDir=0x0,
		.ADCDat=0xf,
		
		// endable ADC 6 and 7
		.ADC6Enable=1,
		.ADC7Enable=1,
		
		// disable PWM34 and PWM56 - four HIZ pins each
		.PWM34Mode=0,
		.PWM56Mode=0,
		
		// four HIZ pins on port B
		.BDir=0,
		.BDat=0
	},
	.edgeData={},
};
#elif PROFILE == RAPIDSTRIKE2
/*
my RS:
	ADC0 = digital pusher idle (pressed when pusher fully retracted)
	ADC1 = digital trigger
	ADC2 = digital rev
	ADC3 = mode switch / 3way. top = 0v, mid = 2.5v, bot = 5v
	ADC4 = front pot
	ADC5 = rear pot
	
	PWM1 = rev
	PWM2 = pusher

*/

//TODO: curve names?


EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(OUT_PWM1,MP_REPLACE,IN_CONSTANT0,0xff,1,SW_TRUE,0),
		//MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0xff,0,SW_TRUE,0),
		
		// rev half way when rev pulled but not firing
		//MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0x80,0,-SW_ADC2,0),
		
		// when firing, even if rev trigger not pulled, rev to about 3/4, and run half RoF
		//MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0xc0,0,SW_FUNC0,0),
		//MIX(OUT_PWM2,MP_REPLACE,IN_ADC5,0x80,0,SW_FUNC0,0),
		
		// when both triggers pulled, rev and pusher both to full speed (set by knob)
		//MIX(OUT_PWM1,MP_REPLACE,IN_ADC4,0xff,0,SW_FUNC1,0),
		//MIX(OUT_PWM2,MP_REPLACE,IN_ADC5,0xff,0,SW_FUNC1,0),
		
		
		// single:
		// start pusher at ADC level when trigger pulled 
		//MIX(OUT_PWM2,MP_REPLACE,IN_ADC5     ,0x90,0,SW_EDGE0,0),
		// brake pusher when fully retracted
		//MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0x00,0,SW_EDGE1,0),
		
		// burst of 3:
		// start pusher at ADC level when trigger pulled 
		MIX(OUT_VAR0,MP_REPLACE,IN_CONSTANT0,0x00,0,SW_EDGE0,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_ADC5     ,0x90,0,SW_EDGE0,0),
		// brake pusher when fully retracted
		MIX(OUT_VAR0,MP_ADD    ,IN_CONSTANT0,0x00,1,SW_EDGE1,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0x00,0,SW_FUNC3,0),
		
		
		
		},
	.curves5={},
	.curves9={},
	.logicData={
		LOGIC( SW_ADC0, OP_DD_OR, -SW_ADC1), 	// 0x21 = OR, output of this should be used to trigger pusher to activate
										// Firing: pusher not retracted or trigger pulled
		LOGIC(SW_FUNC0, OP_DD_AND, -SW_ADC2), 	// 0x20 = AND, Firing && rev pulled, required for full range/speed
										// full: firing and reved 
		LOGIC(OUT_VAR0, OP_AC_GT, 3),			// FUNC2 = var0==3, done with burst
		LOGIC(SW_EDGE1, OP_DD_AND, SW_FUNC2)	// FUNC3 = end of burst
		LOGIC(SW_EDGE0, OP_DD_AND, SW_FUNC2)	// FUNC4 = start of burst  
	},
	.pinData={
		// input and pull-ups for low four ADC pins (fourth is currently unused)
		.ADCDir=0x0,
		.ADCDat=0xf,
		
		// endable ADC 6 and 7
		.ADC6Enable=1,
		.ADC7Enable=1,
		
		// disable PWM34 and PWM56 - four HIZ pins each
		.PWM34Mode=0,
		.PWM56Mode=0,
		
		// four HIZ pins on port B
		.BDir=0,
		.BDat=0
	},
	.edgeData={
		-SW_ADC1,		// EDGE0 = trigger pulled
		-SW_ADC0,		// EDGE1 = pusher fully retracted
	},
};
#elif PROFILE == STRYFE
/*
Tanner stryfe:
	ADC0 = rev pot
	ADC1 = rev mode switch (high = push to rev, low = push to toggle)
	ADC2 = ~rev trigger
	ADC3 = 
	ADC6 = 
	ADC7 = 
	
	PWM1 = flywheels 
	PWM2 = 

	rev to ADC0 when mode=ptr&trigger || mode=ptt&trigger/2

*/

//TODO: curve names?


EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(OUT_PWM1,MP_REPLACE,IN_CONSTANT0,0,1,SW_TRUE,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0,1,SW_TRUE,0),
				
		// rev to knob setting when active 
		MIX(OUT_PWM1,MP_REPLACE,IN_ADC0,0xff,0,SW_FUNC6,0),
			
		// OUT_VAR1 = push-to-toggle state
		// set 0xff on rising edge of first trigger pull, set 0 on falling edge of second
		// reset to 0 when switch taken out of toggle mode
		MIX(OUT_VAR1,MP_REPLACE,IN_CONSTANT0,0x00,0x00,SW_FUNC8,0),
		MIX(OUT_VAR1,MP_REPLACE,IN_CONSTANT0,0x00,0xff,SW_FUNC7,0),
		MIX(OUT_VAR1,MP_REPLACE,IN_CONSTANT0,0x00,0x00, SW_ADC1,0),
		
		// OUT_VAR2 = MIXOUT11 at previous release
		MIX(OUT_VAR2,MP_REPLACE,IN_MIXOUT(OUT_VAR1) ,0xff,0x00,SW_FUNC3,0),
		
		// OUT_VAR0 = previous value of rev trigger, for edge detection.
		MIX(OUT_VAR0,MP_REPLACE,IN_CONSTANT0,0x00,0xff,-SW_ADC2,0),
		MIX(OUT_VAR0,MP_REPLACE,IN_CONSTANT0,0x00,0x00, SW_ADC2,0),
			
		},
	.curves5={},
	.curves9={},
	.logicData={
		LOGIC( -SW_ADC2,			OP_DD_AND,	SW_ADC1),		//  FUNC0 = rev trigger && mode = push-to-rev
		
		LOGIC( IN_MIXOUT(OUT_VAR0),	OP_AC_EQ,		0xff),	 	//  FUNC1 = prev rev trigger (MIXOUT10 == 0xff)
		
		LOGIC( -SW_ADC2,			OP_DD_AND,	-SW_FUNC1), 	//  FUNC2 = rev trigger && not prev rev trigger (not->pulled transition)
		LOGIC(  SW_ADC2,			OP_DD_AND,	SW_FUNC1), 	//  FUNC3 = not rev trigger && prev rev trigger (pulled->not transition)
		
		LOGIC( IN_MIXOUT(OUT_VAR1),	OP_AC_EQ,		0xff),	 	//  FUNC4 = rev toggle state (MIXOUT11 == 0xff)
		
		LOGIC( SW_FUNC4,			OP_DD_AND,	-SW_ADC1),	//  FUNC5 = rev toggle state && mode = push-to-toggle
		
		LOGIC( SW_FUNC5,			OP_DD_OR,		SW_FUNC0),	//  FUNC6 = FUNC5 || FUNC0 == should rev
		
		LOGIC( SW_FUNC2,			OP_DD_AND,	-SW_FUNC4),	//  FUNC7 = FUNC2 && -FUNC4, rising edge while inactive
		LOGIC( SW_FUNC3,			OP_DD_AND,	SW_FUNC9),	//  FUNC8 = FUNC3 &&  FUNC4, falling edge while toggle was active at prev fall edge
		LOGIC( IN_MIXOUT(OUT_VAR2),	OP_AC_EQ,		0xff),	 	//  FUNC9 = rev toggle state at previous falling edge
		
	
	},
	.pinData={
		// input and pull-ups for low four ADC pins
		// all four inputs, pull ups on 1,2. 0 is analog, 3 is unused
		.ADCDir=0x1,
		.ADCDat=0x7,
		
		// endable ADC 6 and 7
		.ADC6Enable=1,
		.ADC7Enable=1,
		
		// disable PWM34 and PWM56 - four HIZ pins each
		.PWM34Mode=0,
		.PWM56Mode=0,
		
		// four HIZ pins on port B
		.BDir=0,
		.BDat=0
	},
	.edgeData={},
};
#elif PROFILE == 3 //STRYFE2, edge detector example
/*
Tanner stryfe:
	ADC0 = rev pot
	ADC1 = rev mode switch (high = push to rev, low = push to toggle)
	ADC2 = ~rev trigger
	ADC3 = 
	ADC6 = 
	ADC7 = 
	
	PWM1 = flywheels 
	PWM2 = 

	rev to ADC0 when mode=ptr&trigger || mode=ptt&trigger/2

*/

//TODO: curve names?


EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(OUT_PWM1,MP_REPLACE,IN_CONSTANT0,0,1,SW_TRUE,0),
		MIX(OUT_PWM2,MP_REPLACE,IN_CONSTANT0,0,1,SW_TRUE,0),
				
		// rev to knob setting when active 
		MIX(OUT_PWM1,MP_REPLACE,IN_ADC0,0xff,0,SW_FUNC4,0),
		
		// OUT_VAR0 = Number of edges, EDGE0 *or* EDGE1. reset on EDGE2
		MIX(OUT_VAR0,MP_ADD,    IN_CONSTANT0,0x00,0x40,SW_FUNC1,0),
		MIX(OUT_VAR0,MP_REPLACE,IN_CONSTANT0,0x00,0x00,SW_EDGE2,0),
		
			
		},
	.curves5={},
	.curves9={},
	.logicData={
		LOGIC( -SW_ADC2,			OP_DD_AND,	SW_ADC1),		//  FUNC0 = rev trigger && mode = push-to-rev
		
		LOGIC( SW_EDGE0,			OP_DD_OR,		SW_EDGE1)		//  FUNC1 = both edges of rev, up or down
		LOGIC( IN_MIXOUT(OUT_VAR0),	OP_AC_NEQ,	0x00)		//  FUNC2 = edge count=0, should rev
		
		LOGIC( SW_FUNC2,			OP_DD_AND,	-SW_ADC1),	//  FUNC3 = rev toggle state && mode = push-to-toggle
		
		LOGIC( SW_FUNC3,			OP_DD_OR,		SW_FUNC0),	//  FUNC4 = FUNC3 || FUNC0 == should rev
		
	
	},
	.pinData={
		// input and pull-ups for low four ADC pins
		// all four inputs, pull ups on 1,2. 0 is analog, 3 is unused
		.ADCDir=0x1,
		.ADCDat=0x7,
		
		// endable ADC 6 and 7
		.ADC6Enable=1,
		.ADC7Enable=1,
		
		// disable PWM34 and PWM56 - four HIZ pins each
		.PWM34Mode=0,
		.PWM56Mode=0,
		
		// four HIZ pins on port B
		.BDir=0,
		.BDat=0
	},
	.edgeData={
		-SW_ADC2,		// EDGE0 = rev pulled, replaces FUNC2
		SW_ADC2,		// EDGE1 = rev released, replaces FUNC3
		-SW_ADC1,		// EDGE2 = mode change to push-to-toggle
	},
};
#endif


void settings_init()
{
	//TODO: add some sanity-check here, and load some sane or nop settings block if eeprom not valid.
	eeprom_read_block(&settings,&ee_settings,sizeof(settings));
}