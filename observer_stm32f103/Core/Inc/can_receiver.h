#ifndef CAN_RECEIVER_H
#define CAN_RECEIVER_H

#include "stm32f1xx_hal.h"

HAL_StatusTypeDef CanReceiver_Init(CAN_HandleTypeDef *hcan);
uint32_t CanReceiver_GetRejectedCount(void);

#endif
