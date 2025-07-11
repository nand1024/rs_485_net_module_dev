/*
 * rs485_net_port.c
 *
 *  Created on: Jul 3, 2025
 *      Author: oleh
 */

#include <usart.h>



void rs485_port_switch_trancieve()
{
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
}

void rs485_port_switch_recieve()
{
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
}

uint8_t rs485_tx_is_proccess() {
	return uart_tx_is_proccess();
}

uint8_t rs485_port_uart_tx(uint8_t data[], uint8_t size_data)
{
	return uart_tx(data, size_data);
}

uint8_t rs485_port_uart_rx(uint8_t data[], uint8_t size_data)
{
	return uart_rx(data, size_data);
}

void rs485_port_delay_ms(uint16_t ms)
{

}
