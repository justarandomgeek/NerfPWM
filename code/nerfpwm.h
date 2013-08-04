#ifdef __ASSEMBLER__

#else  /* !ASSEMBLER */


#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define ADC_8BIT

void adc_init(void);
void wait_read_ADC(void);

#ifdef ADC_8BIT
extern uint8_t adc_val[6];
#else
extern uint16_t adc_val[6];
#endif

extern volatile uint8_t adc_new_data;




void init_twi(void);


#endif