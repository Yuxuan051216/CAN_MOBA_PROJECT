#ifndef GAME_PROTOCOL_H
#define GAME_PROTOCOL_H

#include <stdint.h>

#include "game_types.h"

#define CAN_ID_ROLE_SWITCH        0x001U
#define CAN_ID_GAME_CTRL          0x010U
#define CAN_ID_MASTER_CLAIM_BASE  0x020U
#define CAN_ID_DEATH_EVENT        0x080U
#define CAN_ID_GLOBAL_STATE       0x100U
#define CAN_ID_POSITION_STATE     0x110U
#define CAN_ID_CRYSTAL_ATTACK     0x120U
#define CAN_ID_HERO_STATE_BASE    0x180U
#define CAN_ID_MOVE_INPUT_BASE    0x200U
#define CAN_ID_SKILL_INPUT_BASE   0x300U
#define CAN_ID_SKILL_RESULT_BASE  0x380U
#define CAN_ID_READY_BASE         0x400U
#define CAN_ID_HEARTBEAT_BASE     0x700U

#define HEARTBEAT_FLAG_JOYSTICK_ENABLED  0x01U

void Protocol_Init(void);
void Protocol_Update(void);

void Protocol_SendHeartbeat(void);
void Protocol_SendSkillInput(uint8_t skill_id, uint8_t target_id);
void Protocol_SendSkillResult(uint8_t source_node,
                              uint8_t skill_id,
                              uint8_t result,
                              uint8_t source_input_seq);
void Protocol_SendMoveInput(uint8_t x_dir, uint8_t y_dir);
void Protocol_SendGlobalState(void);
void Protocol_SendPositionState(void);
void Protocol_SendCrystalAttack(uint8_t crystal_id,
                                uint8_t target_node,
                                uint8_t damage,
                                uint8_t hit_seq);
void Protocol_SendDeathEvent(uint8_t dead_node,
                             uint8_t killer_node,
                             uint8_t cooldown_s);
void Protocol_SendRoleSwitch(uint8_t new_master,
                             uint8_t new_player,
                             uint8_t reason,
                             uint8_t target_node,
                             uint8_t cooldown_s);
void Protocol_SendReady(void);
void Protocol_SendMasterClaim(void);
void Protocol_SendHeroState(void);

void Protocol_WriteInt16LE(uint8_t data[2], int16_t value);
int16_t Protocol_ReadInt16LE(const uint8_t data[2]);
void Protocol_HandleRxFrame(const CanFrame_t *frame);

#endif
