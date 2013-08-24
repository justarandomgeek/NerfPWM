#ifndef ADC_H
#define ADC_H

#define ADC_8BIT

#ifdef ADC_8BIT
typedef uint8_t adc_t;
#else
typedef uint8_t adc_t;
#endif

extern volatile adc_t adc_val[6];
extern volatile uint8_t adc_new_data;

void adc_init(void);

#endif