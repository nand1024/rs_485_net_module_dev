/*
 * rs485_net_port.h
 *
 *  Created on: Jul 3, 2025
 *      Author: oleh
 */

#ifndef RS485_MODULE_RS485_NET_PORT_H_
#define RS485_MODULE_RS485_NET_PORT_H_

#define RS485_MODE_MASTER    0

void rs485_port_switch_trancieve();

void rs485_port_switch_recieve();

uint8_t rs485_tx_is_proccess();

void rs485_port_uart_tx(uint8_t data[], uint8_t size_data);

uint8_t rs485_port_uart_rx(uint8_t data[], uint8_t size_data);

void rs485_port_delay_ms(uint16_t ms);

#endif /* RS485_MODULE_RS485_NET_PORT_H_ */
