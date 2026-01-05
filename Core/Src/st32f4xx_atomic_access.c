/**
  ******************************************************************************
  * @file           : stm32f4xx_atomic_acess.c
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

/* Includes ------------------------------------------------------------------*/


/* Private includes ----------------------------------------------------------*/

// Standard includes
#include "stdio.h"
#include "string.h"
#include "stdint.h"
// Project-related includes
#include "stm32f4xx.h"


/* Private typedef -----------------------------------------------------------*/


/* Private define ------------------------------------------------------------*/


/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/


/* Private function prototypes -----------------------------------------------*/



/* Private user code ---------------------------------------------------------*/


/**
  * @brief  The application entry point.
  * @retval int
  */
 uint32_t u32_atomic_inc(volatile uint32_t *addr)
{
    uint32_t old, status;

    do {
        old = __LDREXW(addr);          // Load-exclusive value
        status = __STREXW(old + 1, addr); // Try store-exclusive
    } while (status != 0);             // Retry if exclusive monitor was lost

    return old + 1;                    // Return updated value
}