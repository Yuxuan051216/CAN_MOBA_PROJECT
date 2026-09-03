#ifndef GAME_COMMON_H
#define GAME_COMMON_H

#include <stdint.h>

uint8_t GameCommon_IsValidNode(uint8_t node_id);
uint8_t GameCommon_IsValidSkill(uint8_t skill_id);
uint8_t GameCommon_ClampHp(int16_t hp);
uint32_t GameCommon_SkillCooldownMs(uint8_t skill_id);
uint8_t GameCommon_TimeReached(uint32_t now, uint32_t deadline);
uint8_t GameCommon_RemainingSeconds(uint32_t end_ms, uint32_t now_ms);

#endif
