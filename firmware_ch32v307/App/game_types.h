#ifndef GAME_TYPES_H
#define GAME_TYPES_H

#include <stdint.h>

typedef enum {
    ROLE_NONE = 0,
    ROLE_MASTER = 1,
    ROLE_PLAYER = 2,
    ROLE_COOLDOWN = 3,
    ROLE_BACKUP = 4,
    ROLE_MASTER_PLAYER = 5
} NodeRole_t;

typedef enum {
    HERO_DEAD = 0,
    HERO_ALIVE = 1,
    HERO_COOLDOWN = 2,
    HERO_READY = 3
} HeroState_t;

typedef enum {
    GAME_IDLE = 0,
    GAME_RUNNING = 1,
    GAME_PAUSED = 2,
    GAME_OVER = 3
} GameState_t;

typedef enum {
    SKILL_NONE = 0,
    SKILL_1 = 1,
    SKILL_2 = 2,
    SKILL_3 = 3
} SkillId_t;

typedef enum {
    SKILL_RESULT_ACCEPTED = 1,
    SKILL_RESULT_GAME_NOT_RUNNING = 2,
    SKILL_RESULT_COOLDOWN = 3,
    SKILL_RESULT_NOT_CURRENT_PLAYER = 4,
    SKILL_RESULT_OUT_OF_RANGE = 5
} SkillResult_t;

typedef enum {
    SWITCH_BY_PLAYER_DEATH = 1,
    SWITCH_BY_MASTER_TIMEOUT = 2,
    SWITCH_BY_MANUAL_RESET = 3,
    SWITCH_BY_NODE_RECOVERY = 4
} SwitchReason_t;

typedef enum {
    GAME_CTRL_START = 1,
    GAME_CTRL_PAUSE = 2,
    GAME_CTRL_RESET = 3
} GameControl_t;

typedef struct {
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
} CanFrame_t;

#endif
