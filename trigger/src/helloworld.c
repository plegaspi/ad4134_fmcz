/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

// AD4134 Libraries
#include <sleep.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vitis/ad4134/spi_engine.h"
#include "vitis/ad4134/ad713x.h"
#include "vitis/ad4134/no_os_spi.h"
#include "vitis/ad4134/xilinx_spi.h"
#include "vitis/ad4134/no_os_delay.h"
#include "vitis/ad4134/no_os_gpio.h"
#include "vitis/ad4134/xilinx_gpio.h"
#include "vitis/ad4134/no_os_util.h"
#include "vitis/ad4134/no_os_error.h"
#include "vitis/ad4134/no_os_pwm.h"
#include "vitis/ad4134/axi_pwm_extra.h"
#include "vitis/ad4134/clk_axi_clkgen.h"
#include "vitis/ad4134/axi_dmac.h"

struct no_os_spi_desc *spi_eng_desc;
struct spi_engine_offload_message spi_engine_offload_message;
uint32_t adc_buffer_a[ADC_BUFFER_SIZE] __attribute__((aligned(1024)));
uint32_t adc_buffer_b[ADC_BUFFER_SIZE] __attribute__((aligned(1024)));
uint32_t adc_buffer_len = ADC_BUFFER_SIZE * sizeof(uint32_t);
uint32_t *adc_buffers[2] = {adc_buffer_a, adc_buffer_b};
volatile uint8_t buffer_idx = 0;
//uint32_t adc_buffer_len = VALID_BYTES;

uint32_t i = 0, j;
int32_t ret;
const float lsb = 4.096 / (pow(2, 23));
float data;

int main()
{
    init_platform();

    print("Hello World\n\r");

    struct axi_clkgen *clkgen_4134;
	struct axi_clkgen_init clkgen_4134_init = {
		.base = XPAR_AXI_AD4134_CLKGEN_BASEADDR,
		.name = "ad4134_clkgen",
		.parent_rate = 100000000
	};
	struct ad713x_dev *ad713x_dev_1;
	//struct ad713x_dev *ad713x_dev_2;
	struct ad713x_init_param ad713x_init_param_1;
	uint32_t spi_eng_dma_flg = DMA_LAST | DMA_PARTIAL_REPORTING_EN;
	struct spi_engine_offload_init_param spi_engine_offload_init_param;
	uint32_t spi_eng_msg_cmds[1];
	static struct xil_spi_init_param spi_engine_init_params = {
		.type = SPI_PS,
	};
	struct xil_gpio_init_param gpio_extra_param;
	struct no_os_gpio_init_param ad4134_1_dclkio = {
		.number = GPIO_DCLKIO_1,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct no_os_gpio_init_param ad4134_1_dclkmode = {
		.number = GPIO_DCLKMODE,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct no_os_gpio_init_param ad4134_1_mode = {
		.number = GPIO_MODE_1,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct no_os_gpio_init_param ad4134_1_pnd = {
		.number = GPIO_PDN_1,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct no_os_gpio_init_param ad4134_1_resetn = {
		.number = GPIO_RESETN_1,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct no_os_gpio_init_param ad4134_cs_sync = {
		.number = GPIO_CS_SYNC,
		.platform_ops = &xil_gpio_ops,
		.extra = &gpio_extra_param
	};
	struct spi_engine_init_param spi_eng_init_param  = {
		.type = SPI_ENGINE,
		.spi_engine_baseaddr = AD4134_SPI_ENGINE_BASEADDR,
		.cs_delay = 0,
		.data_width = 32,
		.ref_clk_hz = AD713x_SPI_ENG_REF_CLK_FREQ_HZ
	};
	const struct no_os_spi_init_param spi_eng_init_prm  = {
		.chip_select = AD4134_1_SPI_CS,
		.max_speed_hz = 50000000,
		.mode = NO_OS_SPI_MODE_1,
		.platform_ops = &spi_eng_platform_ops,
		.extra = (void*)&spi_eng_init_param,
	};

	struct no_os_pwm_desc *axi_pwm;
	struct axi_pwm_init_param axi_zed_pwm_init_trigger = {
		.base_addr = XPAR_ODR_GENERATOR_BASEADDR,
		.ref_clock_Hz = 100000000,
		.channel = 0
	};
	struct axi_pwm_init_param axi_zed_pwm_init_odr = {
		.base_addr = XPAR_ODR_GENERATOR_BASEADDR,
		.ref_clock_Hz = 100000000,
		.channel = 1
	};
	struct no_os_pwm_init_param axi_pwm_init_trigger = {
		.period_ns = 3000,//trig_period_ns,
		.duty_cycle_ns = 1,//trig_duty_cycle_ns,
		.phase_ns = 45,
		.platform_ops = &axi_pwm_ops,
		.extra = &axi_zed_pwm_init_trigger
	};
	struct no_os_pwm_init_param axi_pwm_init_odr = {
		.period_ns = 3000,//odr_period_ns,
		.duty_cycle_ns = 120,//odr_duty_cycle_ns,
		.phase_ns = 0,
		.platform_ops = &axi_pwm_ops,
		.extra = &axi_zed_pwm_init_odr
	};

	gpio_extra_param.device_id = GPIO_DEVICE_ID;

	gpio_extra_param.device_id = GPIO_DEVICE_ID;
	gpio_extra_param.type = GPIO_PS;

	ad713x_init_param_1.adc_data_len = ADC_24_BIT_DATA;
	ad713x_init_param_1.clk_delay_en = false;
	ad713x_init_param_1.crc_header = CRC_6;
	ad713x_init_param_1.dev_id = ID_AD4134;
	ad713x_init_param_1.format = QUAD_CH_PO;
	ad713x_init_param_1.gpio_dclkio = &ad4134_1_dclkio;
	ad713x_init_param_1.gpio_dclkmode = &ad4134_1_dclkmode;
	ad713x_init_param_1.gpio_mode = &ad4134_1_mode;
	ad713x_init_param_1.gpio_pnd = &ad4134_1_pnd;
	ad713x_init_param_1.gpio_resetn = &ad4134_1_resetn;
	ad713x_init_param_1.gpio_cs_sync = &ad4134_cs_sync;
	ad713x_init_param_1.mode_master_nslave = false; // Change to false if using external clock
	ad713x_init_param_1.dclkmode_free_ngated = false;
	ad713x_init_param_1.dclkio_out_nin = false;
	ad713x_init_param_1.pnd = true;
	ad713x_init_param_1.spi_init_prm.chip_select = AD4134_1_SPI_CS;
	ad713x_init_param_1.spi_init_prm.device_id = SPI_DEVICE_ID;
	ad713x_init_param_1.spi_init_prm.max_speed_hz = 10000000;
	ad713x_init_param_1.spi_init_prm.mode = NO_OS_SPI_MODE_0;
	ad713x_init_param_1.spi_init_prm.platform_ops = &xil_spi_ops;
	ad713x_init_param_1.spi_init_prm.extra = (void *)&spi_engine_init_params;
	ad713x_init_param_1.spi_common_dev = 0;


	spi_eng_msg_cmds[0] = READ(4);
	Xil_ICacheEnable();
	Xil_DCacheEnable();


	ret = axi_clkgen_init(&clkgen_4134, &clkgen_4134_init);
	if (ret != 0)
		return -1;

	ret = axi_clkgen_set_rate(clkgen_4134, AD713x_SPI_ENG_REF_CLK_FREQ_HZ);
	if (ret != 0)
		return -1;

	ret = no_os_pwm_init(&axi_pwm, &axi_pwm_init_trigger);
	if (ret != 0)
		return ret;

	ret = no_os_pwm_init(&axi_pwm, &axi_pwm_init_odr);
	if (ret != 0)
		return ret;


	ret = ad713x_init(&ad713x_dev_1, &ad713x_init_param_1);
	if (ret != 0)
		return -1;

	spi_engine_offload_init_param.rx_dma_baseaddr = AD4134_DMA_BASEADDR;
	spi_engine_offload_init_param.offload_config = OFFLOAD_RX_EN;
	spi_engine_offload_init_param.dma_flags = spi_eng_dma_flg;

	ret = no_os_spi_init(&spi_eng_desc, &spi_eng_init_prm);
	if (ret != 0)
		return -1;

	ret = spi_engine_offload_init(spi_eng_desc, &spi_engine_offload_init_param);
	if (ret != 0)
		return -1;

	spi_engine_offload_message.commands = spi_eng_msg_cmds;
	spi_engine_offload_message.no_commands = NO_OS_ARRAY_SIZE(spi_eng_msg_cmds);
	spi_engine_offload_message.commands_data = NULL;
	spi_engine_offload_message.rx_addr = (uint32_t)adc_buffers[buffer_idx];
	spi_engine_offload_message.tx_addr = 0xA000000;

	while(1) {
		ret = spi_engine_offload_transfer(spi_eng_desc, spi_engine_offload_message,
										  (AD4134_FMC_CH_NO * AD4134_FMC_SAMPLE_NO));
		if (ret != 0)
				return ret;

		Xil_DCacheInvalidateRange((INTPTR)adc_buffer,
					  AD4134_FMC_SAMPLE_NO * AD4134_FMC_CH_NO *
					  sizeof(uint32_t));

		for (i = 0; i < AD7134_FMC_SAMPLE_NO; i++) {
			j = 0;
			printf("%lu: ", i);
			while (j < AD4134_FMC_CH_NO) {
				adc_buffer[AD4134_FMC_CH_NO * i + j] &= 0xffffff00;
				adc_buffer[AD4134_FMC_CH_NO * i + j] >>= 8;
				data = lsb * (int32_t)adc_buffer[AD4134_FMC_CH_NO * i + j];
				if (data > 4.095)
					data = data - 8.192;
				printf("CH%lu: 0x%08lx = %+1.5fV ", j,
					   adc_buffer[AD4134_FMC_CH_NO * i + j], data);
				if (j == AD4134_FMC_CH_NO)
					printf("\n");
				j++;
			}
	}
    cleanup_platform();
    return 0;
}
