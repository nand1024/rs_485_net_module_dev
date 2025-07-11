/*
 * rs485.h
 *
 *  Created on: Jul 3, 2025
 *      Author: oleh
 */

#ifndef RS485_MODULE_RS485_NET_H_
#define RS485_MODULE_RS485_NET_H_

#include <stdint.h>

#define RS_485_PAYLOAD_SIZE     32
#define RS_485_FIELDS_SIZE		5//(start byte) + (adress) + (size data) + (crc) + (end byte)
#define RS_485_TX_BUFFER_SIZE   (RS_485_PAYLOAD_SIZE + RS_485_FIELDS_SIZE)
#define RS_485_RX_BUFFER_SIZE   (RS_485_TX_BUFFER_SIZE * 2)
#define RS485_BUFFER_SIZE       RS_485_RX_BUFFER_SIZE


typedef enum {
	rs485_in_proccesse,
	rs485_succes,
	rs485_error,
}rs485_ret_type;

typedef enum {
	rs485_wait_start,
	rs485_recieve_adr,
	rs485_recieve_size_data,
	rs485_recieve_data,
	rs485_recieve_crc,
	rs485_recieve_end,
} rs485_stage_rx;


typedef struct {
	uint8_t adress;
	uint8_t data_size;
	uint8_t index_data;
	void *data;
	uint8_t control_crc;
	rs485_stage_rx stage;
} rs485_pack_type;

rs485_ret_type rs485_data_deserialize (uint8_t rx_bytes[], uint8_t size_rx, rs485_pack_type *packet);

rs485_ret_type rs485_data_serialize(uint8_t tx_bytes[], uint8_t size_tx, rs485_pack_type *packet);

rs485_ret_type rs485_get_message(rs485_pack_type *message);

rs485_ret_type rs485_send_message(rs485_pack_type *message);


#endif /* RS485_MODULE_RS485_NET_H_ */
