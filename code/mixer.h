#ifndef MIXER_H
#define MIXER_H
/*
 *	outputs: (0-3F)
 *		0x0#	
 *			0	nop (unused mixer) 	
 *			1-6	Physical PWM channels
 *			7-F	extras for intermediate values (will be one cycle delayed)    
 *		0x1#		controls for display? LED ammo count? LCD screen?
 *		0x2#-0x3#	unassigned
 *		 
 */  

#include "settings.h"

uint8_t read_digital(int8_t logicid);

uint8_t read_analog(uint8_t inputid);

void apply_mix(MixData *mix);
uint8_t apply_curve(uint8_t input, int8_t curveid);

#endif