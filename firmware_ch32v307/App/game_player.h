#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include "game_types.h"

void GamePlayer_Init(void);
void GamePlayer_Update(void);
void GamePlayer_OnGlobalState(const CanFrame_t *frame);

#endif
