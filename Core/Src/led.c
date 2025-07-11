/*
 * led.c
 *
 *  Created on: Jul 8, 2025
 *      Author: oleh
 */
#include "main.h"
#include "cmsis_os.h"
#include "../RS485_module/rs485_user.h"

void led_prc(void *arg)
{
uint8_t led_manage[2] = {0};
uint8_t led_state[2] = {0};
	for(;;) {
		rs485_slave_read_payload(led_manage, sizeof(led_manage));
		if (led_manage[0] == 1) {
			LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_5);
			led_state[0] = 1;
		} else {
			LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_5);
			led_state[0] = 0;
		}

		if (led_manage[1] == 1) {
			LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_15);
			led_state[1] = 1;
		} else {
			LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_15);
			led_state[1] = 0;
		}
		rs485_slave_write_payload(led_state, sizeof(led_state));
		vTaskDelay(pdMS_TO_TICKS(30));
	}
}


void create_led_process()
{

	  BaseType_t net_led_prc;

	  net_led_prc = xTaskCreate(led_prc,
			                     "net led",
								 configMINIMAL_STACK_SIZE,
								 NULL,
								 tskIDLE_PRIORITY,
								 NULL);
}
