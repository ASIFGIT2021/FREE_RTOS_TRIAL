/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// Standard includes
#include "stdio.h"
#include "string.h"
#include "stdbool.h"
#include "stdlib.h"
// Project-related includes
#include "murmurhash.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "sensor_data_acquisition_strategy.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define START_STACKING 0x2000A9D0
#define USE_FREERTOS

#define ACQUIRE_SPINLOCK(spin_lock_flag)  while(__atomic_test_and_set (&(spin_lock_flag), __ATOMIC_ACQUIRE)) {}
#define RELEASE_SPIN_LOCK(spin_lock_flag) __atomic_clear (&spin_lock_flag, __ATOMIC_RELEASE)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char *msg1 = "Asif ikbalNigdi\r\n";
char *key = "abra ka dabra";
uint32_t seed = 0xABCDEF12;
static uint32_t hash;
uint8_t rxbyte[2];

volatile uint32_t atomic_counter = 0;
volatile uint32_t non_atomic_counter = 0;
volatile bool spin_lock_flag = 0;

//Testing uninitialized global pointer
uint32_t *pu32_test_global;

SemaphoreHandle_t xMutex;
SemaphoreHandle_t xPrintfMutex;

stCircuarQueue_t     st_Sensor_Data_Queue;
uint8_t              u8arr_packet [200]; // to acquire sensors data
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */
void vTaskLow(void *pv);
void vTaskMedium(void *pv);
void vTaskHigh(void *pv);

void vTaskTest_1(void *pv);
void vTaskTest_2(void *pv);

void vTask_SpinLock_1 (void *pv);
void vTask_SpinLock_2 (void *pv);

void do_long_work_xsec(uint32_t x);
void check_stack_overflow_at75_watermark (void);
void create_stack_overflow (void);

void vTask_Broadcast_Sensors_Data (void *pv);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern  uint32_t u32_atomic_inc(volatile uint32_t *addr);

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  //printf("In main\n");
  uint32_t *stack_start = (uint32_t*)START_STACKING;
  uint32_t *stack_watermark75_1KB = NULL;

	// Disable all maskable interrupts globally
  __disable_irq();

  stack_watermark75_1KB = stack_start - 192; // water mark at 75% of 1KB stack
  *stack_watermark75_1KB = 0xDEADBEEF;

	// Create mutex
  xMutex = xSemaphoreCreateMutex(); // Supports Priority Inheritance
  //xMutex = xSemaphoreCreateBinary(); // Does NOT supports Priority Inheritance

  if (xMutex == NULL) 
  {
    // Error: not enough heap
    printf("Synchronization object (mutex/semaphore) not created!!\n");
    while(1);
  }

  // switch (xMutex->ucQueueType) // defined in queue.c thus cant not access member ofxMutex by forward definition.
  // {
  //   case queueQUEUE_TYPE_BINARY_SEMAPHORE:
  //     printf ("Binary semaphore created!\n");
  //     break;

  //   case queueQUEUE_TYPE_COUNTING_SEMAPHORE :
  //     printf ("Counting semaphore created!\n");
  //     break;
    
  //   case queueQUEUE_TYPE_MUTEX:
  //     printf ("Mutex created!\n");
  //     break;

  //   case queueQUEUE_TYPE_RECURSIVE_MUTEX:
  //     printf ("Recursive mutex created!\n");
  //     break;

  //   default:
  //     break;  
  // }

  // Make the semaphore **available initially** -- Mutex is bydefault available
  // if (queueQUEUE_TYPE_BINARY_SEMAPHORE == xMutex->ucQueueType)
  // {
  //   xSemaphoreGive(xMutex);
  // }
  
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  printf("In main\n");

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	//HAL_UART_Receive_IT(&huart2, rxbyte, 2); // Notify me when ONE byte arrives into the RDR/DR register
	huart2.Instance->CR1 |= (USART_CR1_RE | USART_CR1_RXNEIE);
	
	// Atomic increment of variables
	u32_atomic_inc(&atomic_counter);

  // Initialize the sensor data acquisition circular queue
  v_Sensor_Data_Queue_Init ( &st_Sensor_Data_Queue );
  
	// Non-cryptographic hash function
	hash = murmurhash (key, strlen(key), seed);
	
	// Create FreeRTOS OBJECTS
	xPrintfMutex = xSemaphoreCreateMutex();
	
	// Tasks creation
  //  xTaskCreate(vTaskLow, "Low", 256, NULL, 1, NULL);
	//  xTaskCreate(vTaskMedium, "Med", 256, NULL, 2, NULL);
	//  xTaskCreate(vTaskHigh, "High", 256, NULL, 3, NULL);

   //xTaskCreate(vTaskTest_1, "vTaskTest_1", 256, NULL, 3, NULL);
   //xTaskCreate(vTaskTest_2, "vTaskTest_2", 256, NULL, 3, NULL);

   xTaskCreate(vTask_SpinLock_1, "vTask_SpinLock_1", 256, NULL, 1, NULL);
   xTaskCreate(vTask_SpinLock_2, "vTask_SpinLock_2", 256, NULL, 2, NULL);
   
   xTaskCreate(vTask_Broadcast_Sensors_Data, "vTask_Broadcast_Sensors_Data", 256, NULL, 2, NULL);
	 
   // Re-enable all interrupts globally
   __enable_irq();

   // Application code
     //Testing uninitialized local pointer
  // uint32_t *pu32_test_local;
  // printf("Uninitialized local pointer pu32_test = %p\n", pu32_test_local);

  // printf("Uninitialized global pointer pu32_test = %p\n", pu32_test_global);

  // Allocate some memory dynamically
  // uint8_t *pu8_mem_alloc = (uint8_t*)malloc(10);
  // memset(pu8_mem_alloc, 0xAA, 10);
  // free (pu8_mem_alloc);
  

  // printf("Dynamically allocated memory address = %u\n", *pu8_mem_alloc);

  // pu8_mem_alloc = NULL;

  // printf("Dynamically allocated memory address = %u\n", *pu8_mem_alloc);
  
  // Start TIMER 1 in output compare (OC) mode
  HAL_TIM_OC_Start_IT (&htim1, TIM_CHANNEL_1); 
  HAL_ADC_Start_IT (&hadc1);

	 // Start the FreeRTOS scheduler
	 //vTaskStartScheduler();
	 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T1_CC1;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 5;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_3;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_8;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_9;
  sConfig.Rank = 4;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 84-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 10000;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_OC_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_OC1REF;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_OC_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GREEN_LED_Pin|ORANGE_LED_Pin|RED_LED_Pin|BLUE_LED_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : CLK_IN_Pin */
  GPIO_InitStruct.Pin = CLK_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(CLK_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GREEN_LED_Pin ORANGE_LED_Pin RED_LED_Pin BLUE_LED_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = GREEN_LED_Pin|ORANGE_LED_Pin|RED_LED_Pin|BLUE_LED_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_SCL_Pin Audio_SDA_Pin */
  GPIO_InitStruct.Pin = Audio_SCL_Pin|Audio_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(MEMS_INT2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
#ifdef USE_FREERTOS

/*------------------------------------------------------------------------------
**********   Different Tasks related to FreeRTOS Applications ******************
--------------------------------------------------------------------------------*/
void vTaskTest_1(void *pv)
{
    while(1)
    {
        // non_atomic_counter = non_atomic_counter + 1;
        atomic_counter = u32_atomic_inc(&atomic_counter);
			
				xSemaphoreTake(xPrintfMutex, portMAX_DELAY);
        // printf("T1 = %d\n", non_atomic_counter);
        printf("T1 = %d\n", atomic_counter);
				xSemaphoreGive(xPrintfMutex);
    }
}

void vTaskTest_2(void *pv)
{
    while(1)
    {
        //non_atomic_counter = non_atomic_counter + 1;
        atomic_counter = u32_atomic_inc(&atomic_counter);
			
				xSemaphoreTake(xPrintfMutex, portMAX_DELAY);
        //printf("T2 = %d\n", non_atomic_counter);
        printf("T2 = %d\n", atomic_counter);
			  xSemaphoreGive(xPrintfMutex);
    }
}

void vTaskLow(void *pv)
{
    while(1)
    {
        printf("Low: Going to take mutex\n");
        xSemaphoreTake(xMutex, portMAX_DELAY);
        printf("Low: Took mutex, doing long work\n");
        HAL_GPIO_TogglePin(ORANGE_LED_GPIO_Port, ORANGE_LED_Pin); // Orange LED

        // Simulate long operation
        //vTaskDelay(pdMS_TO_TICKS(3000));
        do_long_work_xsec(1u); // scheduler will not block this task (NOT GETTING cpu TO ELEAPSE THIS)

        printf("Low: Releasing mutex\n");
        xSemaphoreGive(xMutex);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskMedium(void *pv)
{
    while(1)
    {
        printf("Medium: Running\n");
        // Busy loop to burn CPU
        //for (volatile uint32_t i = 0; i < 1000000; i++);
        HAL_GPIO_TogglePin(GREEN_LED_GPIO_Port, GREEN_LED_Pin); // Green LED
        vTaskDelay(pdMS_TO_TICKS(510));
    }
}

void vTaskHigh(void *pv)
{
    vTaskDelay(pdMS_TO_TICKS(500)); // Ensure Low holds mutex first

    while(1)
    {
      printf("High: Trying to take mutex\n");
      xSemaphoreTake(xMutex, portMAX_DELAY);
      printf("High: Took mutex\n");
      HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin); // Red LED

      xSemaphoreGive(xMutex);
      printf("High: Released mutex\n");
      vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while(1);
}


void vTask_SpinLock_1 (void *pv)
{
  while(1)
  {
    printf("SpinLock_Task_1 acquiring lock\n");
    ACQUIRE_SPINLOCK(spin_lock_flag);
    vTaskDelay(pdMS_TO_TICKS(50));
    printf("SpinLock_Task_1 releasing lock\n");
    RELEASE_SPIN_LOCK(spin_lock_flag);
  }

}

void vTask_SpinLock_2 (void *pv)
{

  while(1)
  {
    vTaskDelay(pdMS_TO_TICKS(5));
    printf("SpinLock_Task_2 acquiring lock\n");
    ACQUIRE_SPINLOCK(spin_lock_flag);
    vTaskDelay(pdMS_TO_TICKS(100));
    printf("SpinLock_Task_2 releasing lock\n");
    RELEASE_SPIN_LOCK(spin_lock_flag);
  }

}

void vTask_Broadcast_Sensors_Data (void *pv)
{
  uint16_t u16_packet_size = 0;
  const TickType_t xPeriod = pdMS_TO_TICKS(10); // 10 ms
  TickType_t xLastWakeTime = xTaskGetTickCount();

  while(1)
  {
    // Form a data packet of maximum 200 bytes
    u16_packet_size = ePacket_Formation(); // Consumes circular queue eleme
    // Send the datat packet to MCU2 
    if (u16_packet_size)
    {
     // while ( send_data_to_mcu2(u8arr_packet, u16_packet_size) != COMM_STATUS_SUCCESS );
    }

    vTaskDelayUntil(&xLastWakeTime, xPeriod);
  }
}
/*------------------------------------------------------------------------------
**********   Different Hooks related to FreeRTOS Applications ******************
--------------------------------------------------------------------------------*/

void vApplicationMallocFailedHook( void )
{
	/* This function will only be called if an API call to create a task, queue
	or semaphore fails because there is too little heap RAM remaining. */
	for( ;; );
}


void vApplicationStackOverflowHook( xTaskHandle *pxTask, signed char *pcTaskName )
{
 /* This function will only be called if a task overflows its stack. Note
 that stack overflow checking does slow down the context switch
 implementation. */
  while(1)
  {
    __asm volatile( "NOP" );
  }
}



void vApplicationIdleHook( void )
{
		/* This function will only be called if there is no other active task to run */
	while(1)
  {
    __asm volatile( "NOP" );
		//HAL_WWDG_Refresh(&hwwdg);
  }
}


void vApplicationTickHook( void )
{
	/* This example does not use the tick hook to perform any processing. */
}

#endif


/********* Some miscellaneous functionalities *********************************/
void do_long_work_xsec(uint32_t x)
{
    TickType_t start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(x*1000))
    {
        // Burn CPU
        __NOP();
    }
}

void check_stack_overflow_at75_watermark (void)
{
  uint32_t *stack_start = (uint32_t*)START_STACKING;
  uint32_t *stack_watermark75_1KB = NULL;

  uint32_t check_mark_stack;

  stack_watermark75_1KB = stack_start - 192; // water mark at 75% of 1KB stack

  check_mark_stack = *stack_watermark75_1KB;

  if ( check_mark_stack != 0xDEADBEEF)
  {
    printf("Stack crosses 75 percentage boundary");
  }

}

void create_stack_overflow (void)
{
  int arr[240] = {0};

  for (int i=0; i<240; i++)
  {
    arr[i] = 5;
  }

  arr[0]++;
  //create_stack_overflow();
}


/************** Callback of an event ******/

/**
  * @brief  Data receive callback on UART 
  * @note   This function is called  when requested number of bytes are received over UART.
  * @param  huart : UART handle ( this can differentiate the UART channels from which we are 
  *         having interrupt.
  * @retval None
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    //process(rx_byte);

    // Critical: restart the interrupt for the next request
    HAL_UART_Receive_IT(huart, rxbyte, 2);
}

/**
  * @brief  HAL_ADC_ConvCpltCallback
  * @note   This function is called  when a ADC channel gets converted..
  * @param  hadc : ADC handle.
  * @retval None
*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
  uint32_t u32_adc_chanenel_cnt = 0;

  static uint8_t rank_index = 0; // Tracks the current Rank in the sequence
  
  rank_index++;
  
  u32_adc_chanenel_cnt = HAL_ADC_GetValue(hadc);

   // A specific configuration
  if ( rank_index == 1)
  {
    vSensor_Data_Sample_Management (1U, &u32_adc_chanenel_cnt, 4);
  }
  else if ( rank_index == 2 )
  {
    vSensor_Data_Sample_Management (3U, &u32_adc_chanenel_cnt, 4);
  }
  else if ( rank_index == 3 )
  {
    vSensor_Data_Sample_Management (8U, &u32_adc_chanenel_cnt, 4);
  }
  else if ( rank_index == 4 )
  {
    vSensor_Data_Sample_Management (9U, &u32_adc_chanenel_cnt, 4);
  }
  else if ( rank_index == 5 )
  {
    vSensor_Data_Sample_Management (11U, &u32_adc_chanenel_cnt, 4);
  }
  else
  {
    rank_index = 0;
  }

  printf("%u \n", u32_adc_chanenel_cnt);

}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
