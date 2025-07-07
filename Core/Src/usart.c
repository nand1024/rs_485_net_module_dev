/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include "cmsis_os.h"


/* USER CODE BEGIN 0 */
#define SIZE_DMA_RX_BUFFER	128
#define SIZE_DMA_TX_BUFFER	64

static uint8_t rx_dma_buffer[SIZE_DMA_RX_BUFFER];


static uint8_t tx_dma_buffer[SIZE_DMA_TX_BUFFER];

/*
static const uint8_t start_byte_m = '!';
static const uint8_t start_byte_s = '$';
static const uint8_t end_byte = '\n';
static const uint8_t my_adress = 1;
*/
typedef enum {
	rs485_in_proccesse,
	rs485_complete,
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
	uint8_t *data;
	uint8_t control_crc;
	rs485_stage_rx stage;
} rs485_pack_type;

uint8_t uart_tx_is_proccess()
{
	if (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_4)) {
		return 1;
	}
	return 0;
}

uint8_t uart_rx(uint8_t data[], uint8_t size_data)
{
	static uint8_t index_rx_read = 0;

	uint8_t i = 0;
	uint8_t size_rx = SIZE_DMA_RX_BUFFER - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_5);

	for (;  index_rx_read != size_rx && i < size_data; i++) {
		data[i] = rx_dma_buffer[index_rx_read];
		if (++index_rx_read >= SIZE_DMA_RX_BUFFER) {
			index_rx_read = 0;
		}
	}

	return i;
}

uint8_t uart_tx(uint8_t data[], uint8_t size_data)
{
	if (uart_tx_is_proccess() == 0 && size_data <= sizeof(tx_dma_buffer)) {
		for (uint8_t i = 0; i < size_data; i++) {
			tx_dma_buffer[i] = data[i];
		}
		LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, sizeof(tx_dma_buffer));
		LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)tx_dma_buffer);
		LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
		return size_data;
	} else {
		return 0;
	}
}


/*
void rs485_switch_trancieve()
{
	LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_12);
}

void rs485_switch_recieve()
{
	LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_12);
}
*/
void DMA1_Channel4_IRQHandler(void)
{
	if (LL_DMA_IsActiveFlag_TC4(DMA1)) {
		LL_DMA_ClearFlag_TC4(DMA1);
		while(!LL_USART_IsActiveFlag_TC(USART1));
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
		//rs485_switch_recieve();
	}
}
/*
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

rs485_ret_type rs485_data_deserialize (uint8_t rx_byte, rs485_pack_type *packet)
{
	rs485_ret_type res = rs485_in_proccesse;
	switch (packet->stage) {

	case rs485_wait_start:
		if (rx_byte == start_byte_m) {
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
			packet->data[packet->index_data] = rx_byte;
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
			res = rs485_complete;
		} else {
			res = rs485_error;
		}
		packet->stage = rs485_wait_start;
		break;

	default:
		packet->stage = rs485_wait_start;
		break;
	}
	return res;
}

rs485_ret_type rs485_data_serialize(uint8_t tx_bytes[], uint8_t size_of_tx_bytes, rs485_pack_type *packet)
{
	uint8_t control_crc = 0;

	if (size_of_tx_bytes < packet->data_size + 5) {
		return rs485_error;
	}

	tx_bytes[0] = start_byte_m;
	control_crc = crc8(tx_bytes[0], 0);

	tx_bytes[1] = packet->adress;
	control_crc = crc8(tx_bytes[1], control_crc);

	tx_bytes[2] = packet->data_size;
	control_crc = crc8(tx_bytes[2], control_crc);

	for(uint8_t i = 0; i < packet->data_size; i++){
		tx_bytes[i + 3] = packet->data[i];
		control_crc = crc8(tx_bytes[i + 3], control_crc);
	}

	tx_bytes[packet->data_size + 3] = control_crc;
	tx_bytes[packet->data_size + 4] = end_byte;

	return rs485_complete;
}


uint8_t _rs485_recieve_procces(void * arg)
{
	uint8_t index_pack = 0;
	uint8_t index_read = 0;
	uint8_t rx_byte;
	uint8_t size_rx_data;
	uint8_t control_crc = 0;
	rs485_stage_rx stage = rs485_wait_start;
	rs485_switch_recieve();
	for(;;){
		while (index_read != SIZE_DMA_RX_BUFFER - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_5)) {
			rx_byte = rx_dma_buffer[index_read];
			switch (stage) {

			case rs485_wait_start:
				if (rx_byte == start_byte_m) {
					control_crc = crc8(rx_byte, 0);
					stage = rs485_recieve_adr;
				}
				break;

			case rs485_recieve_adr:
				if (rx_byte == my_adress) {
					control_crc = crc8(rx_byte, control_crc);
					stage = rs485_recieve_size_data;
				} else {
					stage = rs485_wait_start;
				}
				break;

			case rs485_recieve_size_data:
				if (rx_byte <= SIZE_PACK) {
					size_rx_data = rx_byte;
					index_pack = 0;
					control_crc = crc8(rx_byte, control_crc);
					stage = rs485_recieve_data;
				} else {
					stage = rs485_wait_start;
				}
				break;

			case rs485_recieve_data:
				if (index_pack < size_rx_data) {
					rx_pack_buffer[index_pack] = rx_byte;
					index_pack++;
					control_crc = crc8(rx_byte, control_crc);
				} else {
					if (control_crc == rx_byte) {
						stage = rs485_recieve_end;
					} else {
						stage = rs485_wait_start;
					}
				}
				break;

			case rs485_recieve_crc:
				if (control_crc == rx_byte) {
					stage = rs485_recieve_end;
				} else {
					stage = rs485_wait_start;
				}
				break;

			case rs485_recieve_end:
				if (end_byte == rx_byte) {
					//copy data
					recieve_fl = 1;
				}
				stage = rs485_wait_start;
				break;

			default:
				stage = rs485_wait_start;
				break;

			}

			if (++index_read >= SIZE_DMA_RX_BUFFER) {
				index_read = 0;
			}
		}

		vTaskDelay(pdMS_TO_TICKS(5));
	}
}

void rs485_trancieve_procces( void * arg)
{
	uint8_t control_crc = 0;
	for(;;){
		if (recieve_fl && !LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_4)) {
			rs485_switch_trancieve();
			LL_mDelay(1);
			tx_dma_buffer[0] = start_byte_m;
			control_crc = crc8(tx_dma_buffer[0], 0);
			tx_dma_buffer[1] = my_adress;
			control_crc = crc8(tx_dma_buffer[1], control_crc);
			tx_dma_buffer[2] = SIZE_PACK;
			control_crc = crc8(tx_dma_buffer[2], control_crc);
			for(uint8_t i = 0; i < SIZE_PACK; i++){
				tx_dma_buffer[i+3] = rx_pack_buffer[i];
				control_crc = crc8(tx_dma_buffer[i+3], control_crc);
			}
			tx_dma_buffer[SIZE_PACK+3] = control_crc;
			tx_dma_buffer[SIZE_PACK+4] = end_byte;

			LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, sizeof(tx_dma_buffer));
			LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)tx_dma_buffer);
			LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
			recieve_fl = 0;
		}
		vTaskDelay(pdMS_TO_TICKS(50));
	}
}

void create_rs485_process() {
	  BaseType_t rs485_rx_prc, rs485_tx_prc;

	  rs485_rx_prc = xTaskCreate(rs485_recieve_procces,
			                     "rs485_rx",
								 configMINIMAL_STACK_SIZE,
								 NULL,
								 tskIDLE_PRIORITY,
								 NULL);

	  rs485_tx_prc = xTaskCreate(rs485_trancieve_procces,
			                     "rs485_tx",
								 configMINIMAL_STACK_SIZE,
								 NULL,
								 tskIDLE_PRIORITY+1,
								 NULL);
}*/
/* USER CODE END 0 */

/* LPUART1 init function */

void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  LL_LPUART_InitTypeDef LPUART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetLPUARTClockSource(LL_RCC_LPUART1_CLKSOURCE_PCLK1);

  /* Peripheral clock enable */
  LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_LPUART1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**LPUART1 GPIO Configuration
  PA2   ------> LPUART1_TX
  PA3   ------> LPUART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_2|LL_GPIO_PIN_3;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_8;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  LPUART_InitStruct.BaudRate = 57600;
  LPUART_InitStruct.DataWidth = LL_LPUART_DATAWIDTH_8B;
  LPUART_InitStruct.StopBits = LL_LPUART_STOPBITS_1;
  LPUART_InitStruct.Parity = LL_LPUART_PARITY_NONE;
  LPUART_InitStruct.TransferDirection = LL_LPUART_DIRECTION_TX_RX;
  LPUART_InitStruct.HardwareFlowControl = LL_LPUART_HWCONTROL_NONE;
  LL_LPUART_Init(LPUART1, &LPUART_InitStruct);
  LL_LPUART_Enable(LPUART1);
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}
/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9|LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 DMA Init */

  /* USART1_TX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_4, LL_DMA_REQUEST_2);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_4, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PRIORITY_LOW);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MODE_NORMAL);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_4, LL_DMA_MDATAALIGN_BYTE);

  /* USART1_RX Init */
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_5, LL_DMA_REQUEST_2);

  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_5, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);

  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PRIORITY_HIGH);

  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MODE_CIRCULAR);

  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PERIPH_NOINCREMENT);

  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MEMORY_INCREMENT);

  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_5, LL_DMA_PDATAALIGN_BYTE);

  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_5, LL_DMA_MDATAALIGN_BYTE);

  /* USER CODE BEGIN USART1_Init 1 */

  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)&USART1->RDR);
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_5, (uint32_t)rx_dma_buffer);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_5, sizeof(rx_dma_buffer));
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_5);

  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)&USART1->TDR);
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_4);
  NVIC_EnableIRQ(DMA1_Channel4_IRQn);

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.BaudRate = 9600;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_ConfigAsyncMode(USART1);
  LL_USART_EnableDMAReq_RX(USART1);
  LL_USART_EnableDMAReq_TX(USART1);
  LL_USART_Enable(USART1);
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
