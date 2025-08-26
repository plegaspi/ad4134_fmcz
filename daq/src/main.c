#include <parameters.h>
#include <stdio.h>
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform_config.h"
#include "xil_printf.h"
#include "xtime_l.h"



#if LWIP_IPV6==1
#include "lwip/ip.h"
#else
#if LWIP_DHCP==1
#include "lwip/dhcp.h"
#endif
#endif

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

#include "../../shared_memory_lib/src/include/shared_memory.h"


int main_thread();
void print_echo_app_header();
void echo_application_thread(void *);

void lwip_init();
#if LWIP_IPV6==0
#if LWIP_DHCP==1
extern volatile int dhcp_timoutcntr;
err_t dhcp_start(struct netif *netif);
#endif
#endif

#define THREAD_STACKSIZE 1024

static struct netif server_netif;
struct netif *echo_netif;

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
	xil_printf("Test");
	sys_thread_new("main_thrd", (void(*)(void*))main_thread, 0,
	                THREAD_STACKSIZE,
	                DEFAULT_THREAD_PRIO);
	vTaskStartScheduler();
	while(1);
	Xil_DCacheDisable();
	Xil_ICacheDisable();
	return 0;
}

void network_thread(void *p)
{
    struct netif *netif;
    unsigned char mac_ethernet_address[] = { 0x00, 0x0a, 0x35, 0x00, 0x01, 0x02 };
#if LWIP_IPV6==0
    ip_addr_t ipaddr, netmask, gw;
#if LWIP_DHCP==1
    int mscnt = 0;
#endif
#endif

    netif = &server_netif;

    xil_printf("\r\n\r\n");
    xil_printf("----- lwIP TCP Send-Once Server ------\r\n");

#if LWIP_DHCP==0
    IP4_ADDR(&ipaddr,  192, 168, 1, 10);
    IP4_ADDR(&netmask, 255, 255, 255,  0);
    IP4_ADDR(&gw,      192, 168, 1, 1);
    print_ip_settings(&ipaddr, &netmask, &gw);
#endif

#if LWIP_DHCP==1
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;
#endif

    if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\r\n");
		return;
    }

    netif_set_default(netif);
    netif_set_up(netif);

    sys_thread_new("xemacif_input_thread", (void(*)(void*))xemacif_input_thread, netif,
            THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

#if LWIP_DHCP==1
    dhcp_start(netif);
    while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
	}
#else
    print_echo_app_header();
    xil_printf("\r\n");
    sys_thread_new("echod", echo_application_thread, 0,
		THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
    vTaskDelete(NULL);
#endif
    return;
}

int main_thread()
{
#if LWIP_DHCP==1
	int mscnt = 0;
#endif

	lwip_init();


	sys_thread_new("NW_THRD", network_thread, NULL,
		THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);

#if LWIP_DHCP==1
    while (1) {
		vTaskDelay(DHCP_FINE_TIMER_MSECS / portTICK_RATE_MS);
		if (server_netif.ip_addr.addr) {
			xil_printf("DHCP request success\r\n");
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
			print_echo_app_header();
			xil_printf("\r\n");
			sys_thread_new("echod", echo_application_thread, 0,
					THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
			break;
		}
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS * 2000) {
			xil_printf("ERROR: DHCP request timed out\r\n");
			xil_printf("Configuring default IP of 192.168.1.10\r\n");
			IP4_ADDR(&(server_netif.ip_addr),  192, 168, 1, 10);
			IP4_ADDR(&(server_netif.netmask), 255, 255, 255,  0);
			IP4_ADDR(&(server_netif.gw),  192, 168, 1, 1);
			print_ip_settings(&(server_netif.ip_addr), &(server_netif.netmask), &(server_netif.gw));
			print_echo_app_header();
			xil_printf("\r\n");
			sys_thread_new("echod", echo_application_thread, 0,
					THREAD_STACKSIZE, DEFAULT_THREAD_PRIO);
			break;
		}
	}
#endif
    vTaskDelete(NULL);
    return 0;
}

void print_ip(char *msg, ip_addr_t *ip)
{
	xil_printf(msg);
	xil_printf("%d.%d.%d.%d\n\r", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}
