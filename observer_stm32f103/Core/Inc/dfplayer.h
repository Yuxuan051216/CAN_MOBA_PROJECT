#ifndef DFPLAYER_H
#define DFPLAYER_H

#include <stdint.h>

#include "stm32f1xx_hal.h"

void DFPlayer_Init(UART_HandleTypeDef *huart);
uint8_t DFPlayer_PlayTrack(uint16_t track);
uint8_t DFPlayer_IsReady(void);
void DFPlayer_OnTxComplete(UART_HandleTypeDef *huart);

#endif
