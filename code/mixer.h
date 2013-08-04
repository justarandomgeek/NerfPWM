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

#define PACKED __attribute__((__packed__))

#define MAX_MIXERS	32
#define MAX_CURVE5	8
#define MAX_CURVE9	8
#define MAX_LOGIC	16


typedef struct t_MixData {
	uint8_t destCh:6;		// index to output channel
	uint8_t mltpx:2;		// multiplex method 0=unused, 1=+ 2=* 3=replace
	uint8_t srcRaw;		// index to analog inputs
	uint8_t weight;		// output = input * (weight+1)/256. There's no reason to use a 0 weight, so by adding one we get range 1-256, which makes the divide easy! 
	int8_t  offset;		// output = input + offset 
	int8_t  logic;			// index to digital input, or logic function. negative index is !index. if this condition is false, skip this mix.
	int8_t  curve;			// index to a curve function, 
	
} PACKED MixData;

typedef struct t_LogicData { // Custom Switches data
	int8_t  v1;			// input 1 -  index to switch or analog (determined by func)
	int8_t  v2;			// input 2 -  const, or index to switch or analog (determined by func)
	uint8_t func;			// enum of functions. comparison of analog, logic of switch
} PACKED LogicData;

typedef struct t_EEData {
	MixData   mixData[MAX_MIXERS];
	uint8_t    curves5[MAX_CURVE5][5];
	uint8_t    curves9[MAX_CURVE9][9];
	LogicData   logicData[MAX_LOGIC];
} PACKED EEData;

extern uint8_t mixOuts[0x40];

uint8_t read_logic(int8_t logicid);
uint8_t read_logic_function(int8_t logicid);

uint8_t read_input(uint8_t inputid);

void apply_mix(MixData *mix);
uint8_t apply_curve(uint8_t input, int8_t curveid);