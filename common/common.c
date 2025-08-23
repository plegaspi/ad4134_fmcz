/*#include <stdint.h>
#include "parameters.h"
#include "common.h"

uint32_t adc_buffer_a[ADC_BUFFER_SIZE] __attribute__((section(".shared_dma"), aligned(1024)));
uint32_t adc_buffer_b[ADC_BUFFER_SIZE] __attribute__((section(".shared_dma"), aligned(1024)));
uint32_t adc_buffer_len __attribute__((section(".shared_dma"))) = ADC_BUFFER_SIZE * sizeof(uint32_t);
uint32_t *adc_buffers[2] __attribute__((section(".shared_dma"))) = {adc_buffer_a, adc_buffer_b};
volatile uint8_t buffer_idx __attribute__((section(".shared_dma"))) = 1;
uint8_t digit = 1;*/

#include "common.h"

uint8_t shared_variable = 42;