/*
 * rs485_user.c
 *
 *  Created on: Jul 7, 2025
 *      Author: oleh
 */

#include "rs485_net.h"
#include "rs485_net_port.h"
#include "cmsis_os.h"

static uint8_t buffer_tx[32] = {0};
static uint8_t buffer_rx[32] = {0};

rs485_pack_type message_send = {0};
rs485_pack_type message_get = {0};

void rs485_slave_procces( void * arg)
{
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
		rs485_request_slave_prc (&message_send, &message_get);
		vTaskDelay(pdMS_TO_TICKS(30));
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
