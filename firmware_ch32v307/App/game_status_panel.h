#ifndef GAME_STATUS_PANEL_H
#define GAME_STATUS_PANEL_H

#include <stdint.h>

void GameStatusPanel_Init(void);
void GameStatusPanel_Update(void);
void GameStatusPanel_SetHealth(uint16_t pc_hp, uint16_t board_hp);
void GameStatusPanel_SetGameState(uint8_t game_state);
void GameStatusPanel_OnDeathEvent(uint8_t dead_node, uint8_t cooldown_s);
void GameStatusPanel_OnReset(void);

#endif
