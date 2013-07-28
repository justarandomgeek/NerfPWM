/*
 *	outputs: (0-3F)
 *		0x0#	
 *			0	nop (unused mixer) 	
 *			1-6	Physical PWM channels
 *			7-F	extras for intermediate values (will be one cycle delayed)    
 *		0x1#		controls for display? LED ammo count? LCD screen?
 *		0x2#-0x3#	unassigned
 *		
 *	switches: (0-7F)
 *		0x0#
 *			0-3	PORTB pins
 *			4-8	PWM34 pins if used as digital inputs
 *			9-C	PWM56 pins if used as digital inputs 
 *			D-F	reserved
 *		0x1#		logic functions 
 */  

#define PACKED __attribute__((__packed__))

#define MAX_MIXERS	32
#define MAX_CURVE5	8
#define MAX_CURVE9	8
#define MAX_LOGIC	16


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

void apply_mix(MixData *mix);
int8_t apply_curve(int8_t input, int8_t curveid);