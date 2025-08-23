#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "lwip/sockets.h"
#include "netif/xadapter.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>
#include "xscutimer.h"

#include "../parameters.h"
#include "ad4134/ad713x.h"

#define THREAD_STACKSIZE 1024
#define MAX_CONNECTIONS 1
int new_sd[MAX_CONNECTIONS];

u16_t echo_port = 7;


extern uint32_t *adc_buffers[];
extern uint32_t adc_buffer_len;
extern uint8_t buffer_idx;
extern int fill_buffer(void);

extern uint32_t constant_buffer[ADC_BUFFER_SIZE];

XScuTimer Timer;

void print_echo_app_header(void *arg)
{
    xil_printf("%20s %6d %s\r\n", "streaming server",
                        echo_port,
                        "$ nc <board_ip> 7");
}

void process_stream_request(void *p)
{
    int status;
	u32 start_count, end_count, elapsed, fill_count, fill_elapsed;
	u64 elapsed_time_us, fill_elapsed_time_us;

	u32 timer_freq = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ;

	status = XScuTimer_CfgInitialize(&Timer, XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID), XPAR_XSCUTIMER_0_BASEADDR);
	XScuTimer_EnableAutoReload(&Timer);

	int sd = *(int *)p;

	while (1) {
		XScuTimer_LoadTimer(&Timer, 0xFFFFFFFF);
		XScuTimer_Start(&Timer);
		start_count = XScuTimer_GetCounterValue(&Timer);


		if (fill_buffer() != 0) {
		    xil_printf("SPI/DMA error, aborting send\r\n");
		    break;
		}


		Xil_DCacheInvalidateRange((INTPTR)adc_buffers[buffer_idx], SAMPLE_DATA * sizeof(uint32_t));
		fill_count = XScuTimer_GetCounterValue(&Timer);
		fill_elapsed = start_count - fill_count;
		fill_elapsed_time_us = ((u64)fill_elapsed * 1000000U) / (u64)timer_freq;
		xil_printf("Buffer filled in %lu cycles (%llu us)\r\n", fill_elapsed, fill_elapsed_time_us);

		//xil_printf("Frame size = %d bytes\r\n",
		//           SAMPLE_DATA * sizeof(uint32_t));


		int sent = lwip_send(sd,  (const void *)adc_buffers[buffer_idx], SAMPLE_DATA * sizeof(uint32_t), 0);
		end_count = XScuTimer_GetCounterValue(&Timer);
		XScuTimer_Stop(&Timer);
		elapsed = start_count - end_count;
		elapsed_time_us = ((u64)elapsed * 1000000U) / (u64)timer_freq;
		xil_printf("Transfer completed in %lu cycles (%llu us)\r\n", elapsed, elapsed_time_us);
		xil_printf("Time Between transfer and buffer fill: (%llu us)\r\n",  elapsed_time_us - fill_elapsed_time_us);
		//printf("Sent %d", buffer_idx);
		if (sent <= 0) {
			xil_printf("Client disconnected or error. Closing socket.\r\n");
			break;
		}
		buffer_idx ^= 1;
		//sys_check_timeouts();
		//vTaskDelay(pdMS_TO_TICKS(0));
	}

	lwip_close(sd);
	vTaskDelete(NULL);
}

/*
void process_stream_request(void *p)
{
	int status;
	u32 start_count, end_count, elapsed;
	u64 elapsed_time_us;

	u32 timer_freq = XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ;

	status = XScuTimer_CfgInitialize(&Timer, XScuTimer_LookupConfig(XPAR_PS7_SCUTIMER_0_DEVICE_ID), XPAR_XSCUTIMER_0_BASEADDR);
	XScuTimer_EnableAutoReload(&Timer);

	int sd = *(int *)p;
	while (1) {
		XScuTimer_LoadTimer(&Timer, 0xFFFFFFFF);
		XScuTimer_Start(&Timer);
		start_count = XScuTimer_GetCounterValue(&Timer);

		// Invalidate cache in case buffer is modified by another core/DMA/etc.
		//Xil_DCacheInvalidateRange((INTPTR)constant_buffer, ADC_BUFFER_SIZE * sizeof(uint32_t));
		if (fill_buffer() != 0) {
				    xil_printf("SPI/DMA error, aborting send\r\n");
				    break;
				}

		Xil_DCacheInvalidateRange((INTPTR)adc_buffers[buffer_idx], SAMPLE_DATA * sizeof(uint32_t));

		int sent = lwip_send(sd,  (const void *)constant_buffer, SAMPLE_DATA * sizeof(uint32_t), 0);
		end_count = XScuTimer_GetCounterValue(&Timer);
		XScuTimer_Stop(&Timer);
		elapsed = start_count - end_count;
		elapsed_time_us = ((u64)elapsed * 1000000U) / (u64)timer_freq;
		xil_printf("Task completed in %lu cycles (%llu us)\r\n", elapsed, elapsed_time_us);
		if (sent <= 0) {
			xil_printf("Client disconnected or error. Closing socket.\r\n");
			break;
		}

		// Optional: add delay or yield
		// vTaskDelay(pdMS_TO_TICKS(1));
	}

	lwip_close(sd);

	vTaskDelete(NULL);
}*/


void echo_application_thread()
{
	int sock;
	int size;
	struct sockaddr_in address, remote;

	memset(&address, 0, sizeof(address));

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(echo_port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
		return;

	lwip_listen(sock, 1);
	size = sizeof(remote);

	xil_printf("Waiting for client to stream data...\r\n");

	while (1) {
		new_sd[0] = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size);



		if (new_sd[0] >= 0) {
			int flag = 1;
			lwip_setsockopt(new_sd[0], IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

			xil_printf("Client connected! Starting stream...\r\n");
			sys_thread_new("streambuf", process_stream_request,
				(void*)&(new_sd[0]),
				THREAD_STACKSIZE,
				DEFAULT_THREAD_PRIO);
		}
	}
}
