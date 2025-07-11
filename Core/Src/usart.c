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
#include "../RS485_module/rs485_net.h"
#include "../RS485_module/rs485_net_port.h"
/* USER CODE BEGIN 0 */

static uint8_t rx_dma_buffer[RS_485_RX_BUFFER_SIZE];

static uint8_t tx_dma_buffer[RS_485_TX_BUFFER_SIZE];

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
	uint8_t index_rx_end = RS_485_RX_BUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_5);

	for (;  index_rx_read != index_rx_end && i < size_data; i++) {
		data[i] = rx_dma_buffer[index_rx_read];
		if (++index_rx_read >= RS_485_RX_BUFFER_SIZE) {
			index_rx_read = 0;
		}
	}

	return i;
}

uint8_t uart_tx(uint8_t data[], uint8_t size_data)
{
	if (!LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_4) && size_data <= RS_485_TX_BUFFER_SIZE) {
		for (uint8_t i = 0; i < size_data; i++) {
			tx_dma_buffer[i] = data[i];
		}
		LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_4, size_data);
		LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_4, (uint32_t)tx_dma_buffer);
		LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_4);
		while(!LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_4));
		return size_data;
	} else {
		return 0;
	}
}

void DMA1_Channel4_IRQHandler(void)
{
	if (LL_DMA_IsActiveFlag_TC4(DMA1)) {
		LL_DMA_ClearFlag_TC4(DMA1);
		while(!LL_USART_IsActiveFlag_TC(USART1));
		LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_4);
		rs485_port_switch_recieve();
	}
}

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
