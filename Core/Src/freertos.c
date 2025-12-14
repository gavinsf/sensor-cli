/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "gf_dht11.h"
#include "stdio.h"
#include "gpio.h"
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t ch[1];
extern UART_HandleTypeDef huart2;
/* USER CODE END Variables */
osThreadId MainTaskHandle;
osThreadId ScanfTaskHandle;
osMessageQId QueueHandle;
osSemaphoreId CallScanfTaskHandle;
osSemaphoreId ReleaseQueueHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartMainTask(void const * argument);
void StartScanfTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* Create the semaphores(s) */
  /* definition and creation of CallScanfTask */
  osSemaphoreDef(CallScanfTask);
  CallScanfTaskHandle = osSemaphoreCreate(osSemaphore(CallScanfTask), 1);

  /* definition and creation of ReleaseQueue */
  osSemaphoreDef(ReleaseQueue);
  ReleaseQueueHandle = osSemaphoreCreate(osSemaphore(ReleaseQueue), 1);

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of Queue */
  osMessageQDef(Queue, 255, uint8_t);
  QueueHandle = osMessageCreate(osMessageQ(Queue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of MainTask */
  osThreadDef(MainTask, StartMainTask, osPriorityNormal, 0, 1024);
  MainTaskHandle = osThreadCreate(osThread(MainTask), NULL);

  /* definition and creation of ScanfTask */
  osThreadDef(ScanfTask, StartScanfTask, osPriorityBelowNormal, 0, 1024);
  ScanfTaskHandle = osThreadCreate(osThread(ScanfTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartMainTask */
/**
  * @brief  Function implementing the MainTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartMainTask */
void StartMainTask(void const * argument)
{
  /* USER CODE BEGIN StartMainTask */
	printf("Press User button to start writing\r\n");
	osSemaphoreWait(CallScanfTaskHandle, 0xFFFF);

	/* Infinite loop */
	for(;;)
	{
		osSemaphoreWait(CallScanfTaskHandle, 0xFFFF);
		printf("\r\nInput: \r\n");
		HAL_UART_Receive_IT(&huart2, ch, 1);
		osDelay(1);
	}
  /* USER CODE END StartMainTask */
}

/* USER CODE BEGIN Header_StartScanfTask */
/**
* @brief Function implementing the ScanfTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartScanfTask */
void StartScanfTask(void const * argument)
{
  /* USER CODE BEGIN StartScanfTask */
	uint32_t QueueSize = 0;
	osEvent QueueData;

	osSemaphoreWait(ReleaseQueueHandle, 0xFFFF);

	/* Infinite loop */
	for(;;)
	{
		osSemaphoreWait(ReleaseQueueHandle, 0xFFFF);

		QueueSize = osMessageAvailableSpace(QueueHandle);


		for(uint8_t i = 0; i<=(255-QueueSize); i++)
		{
			QueueData = osMessageGet(QueueHandle, 100);
			HAL_UART_Transmit(&huart2, (uint8_t*)&QueueData.value.v, 1, 4000);
		}

	}
  /* USER CODE END StartScanfTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
int __io_putchar(int ch)
{
  HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
  return ch;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	osSemaphoreRelease(CallScanfTaskHandle);
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(&huart2, ch, 1);
	if (ch[0] == '\r' || ch[0] == '\n')
	{
		osSemaphoreRelease(ReleaseQueueHandle);
	}
	else
	{
		osMessagePut(QueueHandle, ch[0], 200);
	}
}
/* USER CODE END Application */
