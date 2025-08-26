#ifndef __SHARED_MEMORY_H__
#define __SHARED_MEMORY_H__

#include "parameters.h"

//#include <stdint.h>


/* Put into .shared section and align for DMA */
#define SHARED __attribute__((section(".shared"), aligned(1024)))

extern SHARED volatile uint32_t test_value;
/*

extern SHARED uint32_t adc_buffer_a[ADC_BUFFER_SIZE];
extern SHARED uint32_t adc_buffer_b[ADC_BUFFER_SIZE];


extern SHARED volatile uint32_t adc_buffer_len;
extern SHARED volatile uint8_t  buffer_idx;


static inline uint32_t *get_adc_buffer(uint8_t idx) {
    return (idx == 0) ? adc_buffer_a : adc_buffer_b;
}
*/
#endif
