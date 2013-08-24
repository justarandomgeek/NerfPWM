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

enum dest
{
	PWM1	= 0,
	PWM2,
	PWM3,
	PWM4,
	PWM5,
	PWM6,

};

enum multiplex
{
	UNUSED = 0,
	ADD,
	MULTIPLY,
	REPLACE
};


/*	inputs: (0-FF)
 *		0x0#
 *			0-5	ADC 0-5
 *			6	ADC temp?
 *			7	ADC battery reading (using the 1.1 Vbg channel)?
 *			8	% time idle (or busy?)?
 *			9-F	unassigned
 *		0x1#-0xB#	unassigned
 *		0xC#-0xF#	outputs from previous cycle, lower 6 bits = index
 */
enum source
{
	ADC0 = 0,
	ADC1,
	ADC2,
	ADC3,
	ADC4,
	ADC5,
	ADCTEMP,
	
	TIME_BUSY = 0x08,
	
	
	MIXOUT00 = 0xC0,
	MIXOUT01,
	MIXOUT02,
	MIXOUT03,
	MIXOUT04,
	MIXOUT05,
	MIXOUT06,
	MIXOUT07,
	MIXOUT08,
	MIXOUT09,
	MIXOUT0A,
	MIXOUT0B,
	MIXOUT0C,
	MIXOUT0D,
	MIXOUT0E,
	MIXOUT0F,
	MIXOUT10,
	MIXOUT11,
	MIXOUT12,
	MIXOUT13,
	MIXOUT14,
	MIXOUT15,
	MIXOUT16,
	MIXOUT17,
	MIXOUT18,
	MIXOUT19,
	MIXOUT1A,
	MIXOUT1B,
	MIXOUT1C,
	MIXOUT1D,
	MIXOUT1E,
	MIXOUT1F,
	MIXOUT20,
	MIXOUT21,
	MIXOUT22,
	MIXOUT23,
	MIXOUT24,
	MIXOUT25,
	MIXOUT26,
	MIXOUT27,
	MIXOUT28,
	MIXOUT29,
	MIXOUT2A,
	MIXOUT2B,
	MIXOUT2C,
	MIXOUT2D,
	MIXOUT2E,
	MIXOUT2F,
	MIXOUT30,
	MIXOUT31,
	MIXOUT32,
	MIXOUT33,
	MIXOUT34,
	MIXOUT35,
	MIXOUT36,
	MIXOUT37,
	MIXOUT38,
	MIXOUT39,
	MIXOUT3A,
	MIXOUT3B,
	MIXOUT3C,
	MIXOUT3D,
	MIXOUT3E,
	MIXOUT3F,
};

enum logic {
	SW_FALSE=0,
	SW_TRUE,
	
	SW_B4 = 4,
	SW_B5,
	SW_B6,
	SW_B7,
	
	SW_PWM3,
	SW_BRAKE3,
	SW_PWM4,
	SW_BRAKE4,

	SW_PWM5,
	SW_BRAKE5,
	SW_PWM6,
	SW_BRAKE6,
	
	SW_FUNC0,
	SW_FUNC1,
	SW_FUNC2,
	SW_FUNC3,
	SW_FUNC4,
	SW_FUNC5,
	SW_FUNC6,
	SW_FUNC7,
	SW_FUNC8,
	SW_FUNC9,
	SW_FUNCA,
	SW_FUNCB,
	SW_FUNCC,
	SW_FUNCD,
	SW_FUNCE,
	SW_FUNCF,

	SW_ADC0,
	SW_ADC1,
	SW_ADC2,
	SW_ADC3,
	SW_ADC4,
	SW_ADC5,
};


typedef struct t_MixData {
	enum dest 		destCh:6;		// index to output channel
	enum multiplex		mltpx:2;		// multiplex method 0=unused, 1=+ 2=* 3=replace
	enum source		srcRaw;		// index to analog inputs
	uint8_t			weight;		// output = input * (weight+1)/256. There's no reason to use a 0 weight, so by adding one we get range 1-256, which makes the divide easy!
	int8_t			offset;		// output = (weighted)input + offset
	int8_t			logic;		// index to digital input, or logic function. negative index is !index. if this condition is false, skip this mix.
	int8_t			curve;			// index to a curve function,
	
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
extern EEData settings;

uint8_t read_logic(int8_t logicid);
uint8_t read_logic_function(int8_t logicid);

uint8_t read_input(uint8_t inputid);

void apply_mix(MixData *mix);
uint8_t apply_curve(uint8_t input, int8_t curveid);