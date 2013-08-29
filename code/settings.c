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

EEData settings;
static EEMEM EEData ee_settings= {
	.mixData = {
		SIMPLEMIX(PWM1,REPLACE,ADC0),
		SIMPLEMIX(PWM1,ADD,ADC1),
		},
	.curves5={},
	.curves9={},
	.logicData={},
	.pinData={
		.ADCDir=0xf,
		.ADCDat=0xf,
		.ADC6Enable=1,
		.ADC7Enable=1,
		.PWM34Mode=0,
		.PWM56Mode=0,
		.BDir=0,
		.BDat=0xf
	}
};

void settings_init()
{
	eeprom_read_block(&settings,&ee_settings,sizeof(settings));
}