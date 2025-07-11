/*
 * rs485_user.h
 *
 *  Created on: Jul 7, 2025
 *      Author: oleh
 */

#ifndef RS485_MODULE_RS485_USER_H_
#define RS485_MODULE_RS485_USER_H_


void rs485_slave_write_payload(uint8_t payload[], uint8_t size);

void rs485_slave_read_payload(uint8_t payload[], uint8_t size);

void create_rs485_process();

#endif /* RS485_MODULE_RS485_USER_H_ */
