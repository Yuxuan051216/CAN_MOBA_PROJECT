#ifndef MAIN_H
#define MAIN_H

#include "stm32f1xx_hal.h"

#define RUN_LED_GPIO_PORT       GPIOC
#define RUN_LED_PIN             GPIO_PIN_13
#define BUZZER_GPIO_PORT        GPIOB
#define BUZZER_PIN              GPIO_PIN_0

extern CAN_HandleTypeDef hcan;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

void Error_Handler(void);

#endif
