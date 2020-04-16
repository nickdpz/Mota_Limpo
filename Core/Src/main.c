/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "stdint.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#define SIZE_RX 0x10
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */


char CDC_tx_buff[60];//El buffer para enviar los datos por usb
uint8_t CDC_tx_size;//tamaño de buffer
char CDC_rx_flag;//Variable que se modifica en el archivo  usbd_cdc_if.c
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);//Función que se usa para enviar por usbd_cdc_if.c
char rxBufferLora[20];
char txBufferLora[60];
uint8_t sizeBufferLora;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_NVIC_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void RX_Lora()
{
	HAL_Delay(1000);
	HAL_UART_Receive(&huart3, (uint8_t *)&rxBufferLora,SIZE_RX, 200);
	CDC_tx_size=sprintf ("%s Listo",rxBufferLora);
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
}
void Config_Lora ()
{
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"sys get reset\r\n\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"sys get reset:\r\n\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"sys get hweui\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"sys get hweui:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"sys get ver\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"sys get ver:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac get deveui\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac get deveui:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac set nwkskey 297DFB4BE07B4908DDA5D1C2B7509C90\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac set nwkskey 297DFB4BE07B4908DDA5D1C2B7509C90:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac set appskey 57F95428F49CFC3F271A061D05DAB6C7\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac set appskey 57F95428F49CFC3F271A061D05DAB6C7:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac set devaddr 00117EAE\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac set devaddr 00117EAE:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"radio set pwr 20\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"radio set pwr 20:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac save\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac save:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac join abp\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac join abp:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	sizeBufferLora=sprintf(txBufferLora,"mac set dr 4\r\n");
	HAL_UART_Transmit(&huart3, (uint8_t *)&txBufferLora, sizeBufferLora, 100);
	CDC_tx_size=sprintf (CDC_tx_buff,"mac set dr 4:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	RX_Lora();
	HAL_Delay(1000);
	CDC_tx_size=sprintf (CDC_tx_buff,"Ya se configuro RN2903.\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
}

void Ini_RN(){
	HAL_GPIO_WritePin(RESET_LORA_GPIO_Port, RESET_LORA_Pin, 0);
	HAL_Delay(100);
	HAL_GPIO_WritePin(RESET_LORA_GPIO_Port, RESET_LORA_Pin, 1);
	HAL_Delay(100);
	Config_Lora();
}


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USB_DEVICE_Init();
	MX_USART3_UART_Init();

	/* Initialize interrupts */
	MX_NVIC_Init();
	/* USER CODE BEGIN 2 */
	CDC_tx_size=sprintf(CDC_tx_buff,"Begin:\r\n");
	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		HAL_Delay(100);
		HAL_GPIO_WritePin(LED_SENT_GPIO_Port, LED_SENT_Pin, 1);
		HAL_Delay(100);
		HAL_GPIO_WritePin(LED_SENT_GPIO_Port, LED_SENT_Pin, 0);
		CDC_tx_size=sprintf(CDC_tx_buff,"Begin:\r\n");
		CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
	/* USER CODE END WHILE */

	/* USER CODE BEGIN 3 */
	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief NVIC Configuration.
  * @retval None
  */
static void MX_NVIC_Init(void)
{
  /* USB_LP_CAN1_RX0_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 57600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_SENT_GPIO_Port, LED_SENT_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, RESET_LORA_Pin|LED_ENABLED_Pin|TRIGGER_1_Pin|TRIGGER_2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_SENT_Pin */
  GPIO_InitStruct.Pin = LED_SENT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_SENT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SW_Pin ECHO_1_Pin */
  GPIO_InitStruct.Pin = SW_Pin|ECHO_1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : RESET_LORA_Pin LED_ENABLED_Pin TRIGGER_1_Pin TRIGGER_2_Pin */
  GPIO_InitStruct.Pin = RESET_LORA_Pin|LED_ENABLED_Pin|TRIGGER_1_Pin|TRIGGER_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : ECHO_2_Pin */
  GPIO_InitStruct.Pin = ECHO_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ECHO_2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  	CDC_tx_size=sprintf(CDC_tx_buff,"Error\r\n");
  	CDC_Transmit_FS((uint8_t *)&CDC_tx_buff,CDC_tx_size);
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
