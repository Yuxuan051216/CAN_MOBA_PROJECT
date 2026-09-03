#ifndef BSP_LED_H
#define BSP_LED_H

#include <stdint.h>

#include "game_types.h"

void BSP_LED_Init(void);
void BSP_LED_SetRun(uint8_t on);
void BSP_LED_SetRole(uint8_t on);
void BSP_LED_ToggleRun(void);
void BSP_LED_ToggleRole(void);
void BSP_LED_UpdateByRole(NodeRole_t role, HeroState_t hero_state);

#endif
