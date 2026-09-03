#ifndef GAME_MASTER_H
#define GAME_MASTER_H

#include <stdint.h>

#include "game_types.h"

typedef struct {
    uint8_t game_state;
    uint8_t term;

    uint8_t master_node;
    uint8_t embedded_player_node;

    uint8_t pc_hp;
    uint8_t embedded_hp;

    uint8_t pc_score;
    uint8_t embedded_score;
    uint8_t winner;

    uint32_t pc_skill_cd_end[4];
    uint32_t embedded_skill_cd_end[4];
    uint8_t last_skill_input_seq[3];
    uint8_t skill_input_seq_valid[3];

    int16_t pc_x;
    int16_t pc_y;
    int16_t embedded_x;
    int16_t embedded_y;
    int8_t pc_move_x;
    int8_t pc_move_y;
    int8_t embedded_move_x;
    int8_t embedded_move_y;

    uint32_t crystal_last_attack_ms[2];
    uint8_t crystal_in_range[2];
    uint8_t crystal_hit_seq;
    uint32_t pc_respawn_end_ms;

    uint32_t last_global_state_ms;
    uint32_t last_position_state_ms;
    uint32_t last_master_tick_ms;
} GameMasterContext_t;

extern GameMasterContext_t g_game_master;

void GameMaster_Init(void);
void GameMaster_Update(void);
void GameMaster_OnPcSkill(uint8_t skill_id, uint8_t input_seq);
void GameMaster_OnEmbeddedSkill(uint8_t node_id,
                                uint8_t skill_id,
                                uint8_t input_seq);
void GameMaster_OnMoveInput(uint8_t node_id, uint8_t x_dir, uint8_t y_dir);
void GameMaster_OnGameControl(uint8_t command);
void GameMaster_OnGlobalState(const CanFrame_t *frame);
void GameMaster_OnPositionState(const CanFrame_t *frame);
void GameMaster_OnDeathEvent(uint8_t dead_node);
void GameMaster_OnRoleSwitch(uint8_t reason);
void GameMaster_ResetGame(void);

uint8_t GameMaster_GetEmbeddedHp(void);
uint8_t GameMaster_GetPcHp(void);

#endif
