#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <stdint.h>

#include "game_types.h"

void BSP_CAN_Init(void);
uint8_t BSP_CAN_SendStd(uint16_t std_id, const uint8_t *data, uint8_t len);
uint8_t BSP_CAN_Available(void);
uint8_t BSP_CAN_Read(CanFrame_t *frame);

#endif
