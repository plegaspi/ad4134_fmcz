#include <parameters.h>
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_cache.h"

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

/*#define USE_PER_BUFFER_FLAGS
#include "shared_addresses.h"

static inline void dmb_st(void){ __asm__ volatile("dmb ishst" ::: "memory"); }
*/
uint32_t i = 0, j;
int32_t ret;

struct no_os_spi_desc *spi_eng_desc;
struct spi_engine_offload_message spi_engine_offload_message;
/*
static void start_rx_dma(uint32_t sel){
    volatile uint32_t* dst = shm_buf_by_sel(sel);
    Xil_DCacheInvalidateRange((UINTPTR)dst, ADC_BYTES);
    spi_engine_offload_message.rx_addr = (UINTPTR)dst;
    (void)spi_engine_offload_transfer(spi_eng_desc, spi_engine_offload_message, ADC_WORDS);
}

void shm_init_producer(void){
    DMA_SEL    = 0;
    BUF_READY0 = 0;
    BUF_READY1 = 0;
    //start_rx_dma(DMA_SEL);
}

*/


int main()
{
    init_platform();

    print("Hello World\n\r");
    print("Successfully ran Hello World application");
    /*struct axi_clkgen *clkgen_4134;
	struct axi_clkgen_init clkgen_4134_init = {
		.base = XPAR_AXI_AD4134_CLKGEN_BASEADDR,
		.name = "ad4134_clkgen",
		.parent_rate = 100000000
	};
	struct ad713x_dev *ad713x_dev_1;

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
	spi_engine_offload_message.tx_addr = 0xA000000;

	shm_init_producer();

	while (1) {
		uint32_t sel = DMA_SEL;
		volatile uint32_t* dst = shm_buf_by_sel(sel);

		while ( (sel==0 ? BUF_READY0 : BUF_READY1) != 0u ) {
		}

		Xil_DCacheInvalidateRange((UINTPTR)dst, ADC_BYTES);

		spi_engine_offload_message.rx_addr = (UINTPTR)dst;
		ret = spi_engine_offload_transfer(spi_eng_desc, spi_engine_offload_message, ADC_WORDS);
		if (ret) break;

		dmb_st();
		if (sel==0) BUF_READY0 = 1u; else BUF_READY1 = 1u;

		uint32_t n = shm_other(sel);

		if ((n==0 && BUF_READY0==0u) || (n==1 && BUF_READY1==0u)) {
			DMA_SEL = n;
		}

	}
    */
    cleanup_platform();
    return 0;
}
