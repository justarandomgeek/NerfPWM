#ifndef SETTINGS_H
#define SETTINGS_H

#define PACKED __attribute__((__packed__))

#define MAX_MIXERS	32
#define MAX_CURVE5	8
#define MAX_CURVE9	8
#define MAX_LOGIC	16

enum analog_out
{
	OUT_PWM1	= 0,
	OUT_PWM2,
	OUT_PWM3,
	OUT_PWM4,
	OUT_PWM5,
	OUT_PWM6,

};

enum multiplex
{
	MP_UNUSED = 0,
	MP_ADD,
	MP_MULTIPLY,
	MP_REPLACE
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
enum analog_in
{
	IN_ADC0 = 0,
	IN_ADC1,
	IN_ADC2,
	IN_ADC3,
	IN_ADC4,
	IN_ADC5,
	IN_ADCTEMP,
	
	IN_TIME_BUSY = 0x08,
	IN_CONSTANT0 = 0x09,
	
	
	IN_MIXOUTS = 0xC0,
	IN_MIXOUT3F = 0xC0 + 0x3F,
};

#define IN_MIXOUT(n) (IN_MIXOUTS + (n))


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

enum logic_op {
	OP_AC_GT = 0x00,
	OP_AC_LT,
	OP_AC_EQ,
	OP_AC_NEQ,
			
	OP_AA_GT = 0x10,
	OP_AA_LT,
	OP_AA_EQ,
	OP_AA_NEQ,
	
	
	OP_DD_AND = 0x20,
	OP_DD_OR,
	OP_DD_XOR,
};


typedef struct t_MixData {
	enum analog_out	destCh:6;		// index to output channel
	enum multiplex		mltpx:2;		// multiplex method 0=unused, 1=+ 2=* 3=replace
	enum analog_in		srcRaw;		// index to analog inputs
	uint8_t			weight;		// output = input * (weight+1)/256. There's no reason to use a 0 weight, so by adding one we get range 1-256, which makes the divide easy!
	int8_t			offset;		// output = (weighted)input + offset
	int8_t			logic;		// index to digital input, or logic function. negative index is !index. if this condition is false, skip this mix.
	int8_t			curve;		// index to a curve function,
	
} PACKED MixData;

typedef struct t_LogicData { // Custom Switches data
	int8_t  v1;			// input 1 -  index to switch or analog (determined by func)
	int8_t  v2;			// input 2 -  const, or index to switch or analog (determined by func)
	uint8_t func;			// enum of functions. comparison of analog, logic of switch
} PACKED LogicData;

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


typedef struct t_EEData {
	MixData		mixData[MAX_MIXERS];
	uint8_t		curves5[MAX_CURVE5][5];
	uint8_t		curves9[MAX_CURVE9][9];
	LogicData		logicData[MAX_LOGIC];
	PinData		pinData;
} PACKED EEData;

extern uint8_t mixOuts[0x40];

extern EEData settings;

void settings_init(void);

#endif