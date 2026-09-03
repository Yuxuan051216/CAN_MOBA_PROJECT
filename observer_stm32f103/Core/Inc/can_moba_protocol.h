#ifndef CAN_MOBA_PROTOCOL_H
#define CAN_MOBA_PROTOCOL_H

#include <stdint.h>

#define NODE_ID_PC                         0U
#define NODE_ID_BOARD_A                    1U
#define NODE_ID_BOARD_B                    2U
#define NODE_COUNT                         3U

#define CAN_ID_ROLE_SWITCH                 0x001U
#define CAN_ID_GAME_CTRL                   0x010U
#define CAN_ID_DEATH_EVENT                 0x080U
#define CAN_ID_GLOBAL_STATE                0x100U
#define CAN_ID_POSITION_STATE              0x110U
#define CAN_ID_CRYSTAL_ATTACK              0x120U
#define CAN_ID_SKILL_RESULT_BASE           0x380U
#define CAN_ID_SKILL_RESULT_BOARD_A        0x381U
#define CAN_ID_SKILL_RESULT_BOARD_B        0x382U

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
    GAME_CTRL_START = 1,
    GAME_CTRL_PAUSE = 2,
    GAME_CTRL_RESET = 3
} GameControl_t;

typedef enum {
    CRYSTAL_BLUE = 1,
    CRYSTAL_RED = 2
} CrystalId_t;

typedef struct {
    uint16_t id;
    uint8_t dlc;
    uint8_t data[8];
} ObserverCanFrame;

uint8_t ObserverProtocol_IsObservedId(uint16_t id);
uint8_t ObserverProtocol_IsBoardNode(uint8_t node_id);

#endif
