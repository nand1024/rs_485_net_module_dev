/*
 * rs485_user.c
 *
 *  Created on: Jul 7, 2025
 *      Author: oleh
 */

#include "rs485_net.h"
#include "rs485_net_port.h"
#include "cmsis_os.h"
#include "string.h"
static uint8_t buffer_tx[RS_485_PAYLOAD_SIZE] = {0};
static uint8_t buffer_rx[RS_485_PAYLOAD_SIZE] = {0};

rs485_pack_type message_send = {0};
rs485_pack_type message_get = {0};

void rs485_slave_write_payload(uint8_t payload[], uint8_t size)
{
	memcpy(message_send.data, (void *)payload, size);
}

void rs485_slave_read_payload(uint8_t payload[], uint8_t size)
{
	memcpy((void *)payload, message_get.data, size);
}

void rs485_slave_procces( void * arg)
{
	rs485_ret_type res;

	message_get.adress = 1;
	message_get.data = buffer_rx;
	message_get.data_size = sizeof(buffer_rx);
	message_get.index_data = 0;
	message_get.stage = rs485_wait_start;

	message_send.adress = 1;
	message_send.data = buffer_tx;
	message_send.data_size = sizeof(buffer_tx);
	message_send.index_data = 0;
	message_send.stage = rs485_wait_start;

	rs485_port_switch_recieve();

	for(;;){

		res = rs485_get_message(&message_get);
		if (res == rs485_succes) {
			//for echo test
			for(uint8_t x = 0; x < message_send.data_size; x++) {
				*((uint8_t *)message_send.data + x) = *((uint8_t *)message_get.data + x);
			}
			//
			vTaskDelay(pdMS_TO_TICKS(5));
			rs485_send_message(&message_send);
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}

void create_rs485_process() {

	  BaseType_t rs485_slave_prc;

	  rs485_slave_prc = xTaskCreate(rs485_slave_procces,
			                     "rs485_slave",
								 configMINIMAL_STACK_SIZE,
								 NULL,
								 tskIDLE_PRIORITY,
								 NULL);
}
