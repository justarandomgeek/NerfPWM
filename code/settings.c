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

/*
my RS:
	ADC0 = digital pusher idle
	ADC1 = digital trigger
	ADC2 = digital rev
	ADC3 = unused
	ADC6 = front pot
	ADC7 = rear pot
	
	PWM1 = rev
	PWM2 = pusher

*/

//TODO: curve names?


EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		// set up base valuse as floating outputs...
		MIX(PWM1,REPLACE,CONSTANT0,0xff,1,SW_TRUE,0),
		MIX(PWM2,REPLACE,CONSTANT0,0xff,0,SW_TRUE,0),
		
		// rev part way when rev trigger pulled
		MIX(PWM1,REPLACE,ADC6,0xc0,0,SW_ADC2,0),
		
		// rev to full (knob value) when firing, even if rev not pulled
		MIX(PWM1,REPLACE,ADC6,0xff,0,SW_FUNC0,0),
		
		// run pusher slow when firing without first pulling rev trigger
		MIX(PWM2,REPLACE,ADC7,0xc0,0,SW_FUNC0,0),
		
		// run pusher at full knob speed if rev and firing both active
		MIX(PWM2,REPLACE,ADC7,0xff,0,SW_FUNC1,0),
		
		
		},
	.curves5={},
	.curves9={},
	.logicData={
		LOGIC(-SW_ADC0, 0x20, SW_ADC1), // 0x20 = AND, output of this should be used to trigger pusher to activate
		LOGIC(SW_FUNC0, 0x20, SW_ADC2), // 0x20 = AND, Firing && rev pulled, required for full range/speed
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
	}
};

void settings_init()
{
	//TODO: add some sanity-check here, and load some sane or nop settings block if eeprom not valid.
	eeprom_read_block(&settings,&ee_settings,sizeof(settings));
}