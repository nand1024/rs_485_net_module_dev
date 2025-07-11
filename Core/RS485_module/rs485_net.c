/*
 * rs485.c
 *
 *  Created on: Jul 3, 2025
 *      Author: oleh
 */

/* USER CODE BEGIN 0 */
#include <stdint.h>
#include "rs485_net.h"
#include "rs485_net_port.h"

static uint8_t buffer[RS485_BUFFER_SIZE] = {0};

static const uint8_t start_byte_m = '!';
static const uint8_t start_byte_s = '$';
static const uint8_t end_byte = '\n';

uint8_t crc8(uint8_t data, uint8_t crc)
{
    uint8_t poly = 0x07;
	crc ^= data;
	for (uint8_t j = 0; j < 8; j++) {
		if (crc & 0x80)
			crc = (crc << 1) ^ poly;
		else
			crc <<= 1;
	}
    return crc;
}

rs485_ret_type rs485_data_deserialize (uint8_t rx_bytes[], uint8_t size_rx, rs485_pack_type *packet)
{
	rs485_ret_type res = rs485_in_proccesse;
	uint8_t rx_byte;
	for (uint8_t i = 0; i < size_rx; i++) {
		rx_byte = rx_bytes[i];
		switch (packet->stage) {

		case rs485_wait_start:
#if RS485_MODE_MASTER
			if (rx_byte == start_byte_s) {
#else
			if (rx_byte == start_byte_m) {
#endif
				packet->control_crc = crc8(rx_byte, 0);
				packet->stage = rs485_recieve_adr;
			}
			break;

		case rs485_recieve_adr:
			if (rx_byte == packet->adress) {
				packet->control_crc = crc8(rx_byte, packet->control_crc);
				packet->stage = rs485_recieve_size_data;
			} else {
				packet->stage = rs485_wait_start;
			}
			break;

		case rs485_recieve_size_data:
			if (rx_byte == packet->data_size) {
				packet->index_data = 0;
				packet->control_crc = crc8(rx_byte, packet->control_crc);
				packet->stage = rs485_recieve_data;
			} else {
				packet->stage = rs485_wait_start;
				res = rs485_error;
			}
			break;

		case rs485_recieve_data:
			if (packet->index_data < packet->data_size) {
				*((uint8_t*)packet->data + packet->index_data) = rx_byte;
				packet->index_data++;
				packet->control_crc = crc8(rx_byte, packet->control_crc);
			} else {
				if (packet->control_crc == rx_byte) {
					packet->stage = rs485_recieve_end;
				} else {
					packet->stage = rs485_wait_start;
					res = rs485_error;
				}
			}
			break;

		case rs485_recieve_end:
			if (end_byte == rx_byte) {
				res = rs485_succes;
			} else {
				res = rs485_error;
			}
			packet->stage = rs485_wait_start;
			break;

		default:
			packet->stage = rs485_wait_start;
			break;
		}
		if(res != rs485_in_proccesse) {
			break;
		}
	}
	return res;
}

rs485_ret_type rs485_data_serialize(uint8_t tx_bytes[], uint8_t size_tx, rs485_pack_type *packet)
{
	uint8_t control_crc = 0;

	if (size_tx < packet->data_size + RS_485_FIELDS_SIZE) {
		return rs485_error;
	}

#if RS485_MODE_MASTER
	tx_bytes[0] = start_byte_m;
#else
	tx_bytes[0] = start_byte_s;
#endif
	control_crc = crc8(tx_bytes[0], 0);

	tx_bytes[1] = packet->adress;
	control_crc = crc8(tx_bytes[1], control_crc);

	tx_bytes[2] = packet->data_size;
	control_crc = crc8(tx_bytes[2], control_crc);

	for(uint8_t i = 0; i < packet->data_size; i++){
		tx_bytes[i + 3] = *((uint8_t*)packet->data + i);
		control_crc = crc8(tx_bytes[i + 3], control_crc);
	}

	tx_bytes[packet->data_size + 3] = control_crc;
	tx_bytes[packet->data_size + 4] = end_byte;

	return rs485_succes;
}



rs485_ret_type rs485_send_message(rs485_pack_type *message)
{
	rs485_ret_type res;
	rs485_port_switch_trancieve();
	res = rs485_data_serialize(buffer, sizeof(buffer), message);
	if (res == rs485_succes) {
		rs485_port_uart_tx(buffer, message->data_size + RS_485_FIELDS_SIZE);
	}
	return res;
}

rs485_ret_type rs485_get_message(rs485_pack_type *message)
{
	uint8_t rx_size = 0;
	rs485_ret_type res;
	if (rs485_tx_is_proccess() == 0) {
		rs485_port_switch_recieve();
	} else {
		return rs485_in_proccesse;
	}
	rx_size = rs485_port_uart_rx(buffer, sizeof(buffer));
	res = rs485_data_deserialize(buffer, rx_size, message);
	return res;
}

