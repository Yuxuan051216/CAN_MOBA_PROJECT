#include "game_protocol.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_can.h"
#include "bsp_time.h"
#include "game_common.h"
#include "game_master.h"
#include "game_player.h"
#include "game_status_panel.h"
#include "node_role.h"

static uint32_t last_heartbeat_ms;
static uint8_t input_seq;
static uint8_t heartbeat_seq;

static uint8_t Protocol_Send(uint16_t can_id, uint8_t data[8])
{
    return BSP_CAN_SendStd(can_id, data, 8U);
}

static uint8_t Protocol_IdMatchesNode(uint16_t id,
                                      uint16_t base,
                                      uint8_t node_id)
{
    return (uint8_t)(id == (uint16_t)(base + node_id));
}

void Protocol_WriteInt16LE(uint8_t data[2], int16_t value)
{
    uint16_t raw = (uint16_t)value;

    data[0] = (uint8_t)(raw & 0xFFU);
    data[1] = (uint8_t)(raw >> 8);
}

int16_t Protocol_ReadInt16LE(const uint8_t data[2])
{
    uint16_t raw;

    raw = (uint16_t)data[0] |
          (uint16_t)((uint16_t)data[1] << 8);
    return (int16_t)raw;
}

void Protocol_Init(void)
{
    last_heartbeat_ms = millis();
    input_seq = 0U;
    heartbeat_seq = 0U;
}

void Protocol_Update(void)
{
    uint32_t now = millis();

    if((uint32_t)(now - last_heartbeat_ms) >= HEARTBEAT_PERIOD_MS)
    {
        last_heartbeat_ms = now;
        Protocol_SendHeartbeat();
    }
}

void Protocol_SendHeartbeat(void)
{
    uint8_t data[8];

    data[0] = NODE_ID;
    data[1] = (uint8_t)NodeRole_GetHeartbeatRole();
    data[2] = (uint8_t)g_node_role.self_hero_state;
    data[3] = GameMaster_GetEmbeddedHp();
    data[4] = g_node_role.current_term;
    data[5] = g_node_role.cooldown_remaining_s;
    data[6] = USE_JOYSTICK ?
              HEARTBEAT_FLAG_JOYSTICK_ENABLED :
              0U;
    /* 心跳序号用于区分应用层新帧与 CAN 控制器重复发送。 */
    data[7] = heartbeat_seq++;

    Protocol_Send((uint16_t)(CAN_ID_HEARTBEAT_BASE + NODE_ID), data);
}

void Protocol_SendSkillInput(uint8_t skill_id, uint8_t target_id)
{
    uint8_t data[8];
    uint8_t sequence;
    uint16_t can_id;
    uint8_t tx_ok;

    if(!GameCommon_IsValidSkill(skill_id))
    {
        return;
    }

    sequence = input_seq++;
    data[0] = NODE_ID;
    data[1] = skill_id;
    data[2] = target_id;
    data[3] = 0U;
    data[4] = 0U;
    data[5] = sequence;
    data[6] = 0U;
    data[7] = 0U;

    can_id = (uint16_t)(CAN_ID_SKILL_INPUT_BASE + NODE_ID);
    tx_ok = Protocol_Send(can_id, data);
    printf("Skill TX node=%u skill=%u target=%u CAN_ID=0x%03X result=%s\r\n",
           (unsigned int)NODE_ID,
           (unsigned int)skill_id,
           (unsigned int)target_id,
           (unsigned int)can_id,
           tx_ok ? "OK" : "FAIL");

    /*
     * Master+Player 退化模式下 CAN 控制器通常不会接收自身发送帧，
     * 因此本机按键需要直接交给同一套 Master 裁判逻辑处理。
     */
    if(NodeRole_IsMaster() && NodeRole_IsPlayer())
    {
        GameMaster_OnEmbeddedSkill(NODE_ID, skill_id, sequence);
    }
}

void Protocol_SendSkillResult(uint8_t source_node,
                              uint8_t skill_id,
                              uint8_t result,
                              uint8_t source_input_seq)
{
    uint8_t data[8];

    if(!NodeRole_IsMaster())
    {
        return;
    }

    data[0] = NODE_ID;
    data[1] = source_node;
    data[2] = skill_id;
    data[3] = result;
    data[4] = source_input_seq;
    data[5] = g_game_master.pc_hp;
    data[6] = g_game_master.embedded_hp;
    data[7] = g_node_role.current_term;

    Protocol_Send((uint16_t)(CAN_ID_SKILL_RESULT_BASE + NODE_ID), data);
}

void Protocol_SendMoveInput(uint8_t x_dir, uint8_t y_dir)
{
    uint8_t data[8];
    uint8_t sequence;

    if((x_dir > 2U) || (y_dir > 2U))
    {
        return;
    }

    sequence = input_seq++;
    data[0] = NODE_ID;
    data[1] = x_dir;
    data[2] = y_dir;
    data[3] = sequence;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send((uint16_t)(CAN_ID_MOVE_INPUT_BASE + NODE_ID), data);

    if(NodeRole_IsMaster() && NodeRole_IsPlayer())
    {
        GameMaster_OnMoveInput(NODE_ID, x_dir, y_dir);
    }
}

void Protocol_SendGlobalState(void)
{
    uint8_t data[8];

    data[0] = g_node_role.current_term;
    data[1] = g_node_role.current_master;
    data[2] = g_node_role.current_player;
    data[3] = g_game_master.pc_hp;
    data[4] = g_game_master.embedded_hp;
    data[5] = g_game_master.pc_score;
    data[6] = g_game_master.embedded_score;
    data[7] = g_game_master.game_state;

    Protocol_Send(CAN_ID_GLOBAL_STATE, data);
#if NODE_ID == NODE_ID_BOARD_A
    GameStatusPanel_SetHealth(data[3], data[4]);
    GameStatusPanel_SetGameState(data[7]);
#endif
}

void Protocol_SendPositionState(void)
{
    uint8_t data[8];

    if(!NodeRole_IsMaster())
    {
        return;
    }

    Protocol_WriteInt16LE(&data[0], g_game_master.pc_x);
    Protocol_WriteInt16LE(&data[2], g_game_master.pc_y);
    Protocol_WriteInt16LE(&data[4], g_game_master.embedded_x);
    Protocol_WriteInt16LE(&data[6], g_game_master.embedded_y);
    Protocol_Send(CAN_ID_POSITION_STATE, data);
}

void Protocol_SendCrystalAttack(uint8_t crystal_id,
                                uint8_t target_node,
                                uint8_t damage,
                                uint8_t hit_seq)
{
    uint8_t data[8];

    if(!NodeRole_IsMaster())
    {
        return;
    }

    data[0] = crystal_id;
    data[1] = target_node;
    data[2] = damage;
    data[3] = hit_seq;
    data[4] = g_game_master.pc_hp;
    data[5] = g_game_master.embedded_hp;
    data[6] = g_node_role.current_term;
    data[7] = 0U;

    Protocol_Send(CAN_ID_CRYSTAL_ATTACK, data);
}

void Protocol_SendDeathEvent(uint8_t dead_node,
                             uint8_t killer_node,
                             uint8_t cooldown_s)
{
    uint8_t data[8];

    data[0] = dead_node;
    data[1] = killer_node;
    data[2] = cooldown_s;
    data[3] = g_node_role.current_term;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send(CAN_ID_DEATH_EVENT, data);
#if NODE_ID == NODE_ID_BOARD_A
    GameStatusPanel_OnDeathEvent(dead_node, cooldown_s);
#endif
}

void Protocol_SendRoleSwitch(uint8_t new_master,
                             uint8_t new_player,
                             uint8_t reason,
                             uint8_t target_node,
                             uint8_t cooldown_s)
{
    uint8_t data[8];

    data[0] = g_node_role.current_term;
    data[1] = new_master;
    data[2] = new_player;
    data[3] = reason;
    data[4] = target_node;
    data[5] = cooldown_s;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send(CAN_ID_ROLE_SWITCH, data);
#if NODE_ID == NODE_ID_BOARD_A
    if(reason == SWITCH_BY_MANUAL_RESET)
    {
        GameStatusPanel_OnReset();
    }
#endif
}

void Protocol_SendReady(void)
{
    uint8_t data[8];

    data[0] = NODE_ID;
    data[1] = 1U;
    data[2] = g_node_role.current_term;
    data[3] = 0U;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send((uint16_t)(CAN_ID_READY_BASE + NODE_ID), data);
}

void Protocol_SendMasterClaim(void)
{
    uint8_t data[8];
    uint8_t claim_term;

    claim_term = g_node_role.election_active ?
                 g_node_role.election_term :
                 (uint8_t)(g_node_role.current_term + 1U);

    data[0] = NODE_ID;
    data[1] = claim_term;
    data[2] = NODE_ID;
    data[3] = (uint8_t)g_node_role.self_hero_state;
    data[4] = 0U;
    data[5] = 0U;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send((uint16_t)(CAN_ID_MASTER_CLAIM_BASE + NODE_ID), data);
}

void Protocol_SendHeroState(void)
{
    uint8_t data[8];

    data[0] = NODE_ID;
    data[1] = GameMaster_GetEmbeddedHp();
    data[2] = (uint8_t)g_node_role.self_hero_state;
    data[3] = (uint8_t)NodeRole_GetHeartbeatRole();
    data[4] = g_node_role.current_term;
    data[5] = g_node_role.cooldown_remaining_s;
    data[6] = 0U;
    data[7] = 0U;

    Protocol_Send((uint16_t)(CAN_ID_HERO_STATE_BASE + NODE_ID), data);
}

void Protocol_HandleRxFrame(const CanFrame_t *frame)
{
    uint8_t node_id;

    if((frame == 0) || (frame->dlc != 8U))
    {
        return;
    }

    if(frame->id == CAN_ID_ROLE_SWITCH)
    {
        NodeRole_OnRoleSwitch(frame->data[0],
                              frame->data[1],
                              frame->data[2],
                              frame->data[3],
                              frame->data[4],
                              frame->data[5]);
        GameMaster_OnRoleSwitch(frame->data[3]);
#if NODE_ID == NODE_ID_BOARD_A
        if(frame->data[3] == SWITCH_BY_MANUAL_RESET)
        {
            GameStatusPanel_OnReset();
        }
#endif
        return;
    }

    if(frame->id == CAN_ID_GAME_CTRL)
    {
        GameMaster_OnGameControl(frame->data[0]);
        return;
    }

    if(frame->id == CAN_ID_DEATH_EVENT)
    {
        NodeRole_OnDeathEvent(frame->data[0], frame->data[2]);
        GameMaster_OnDeathEvent(frame->data[0]);
#if NODE_ID == NODE_ID_BOARD_A
        GameStatusPanel_OnDeathEvent(frame->data[0], frame->data[2]);
#endif
        printf("Death event: dead=%u killer=%u cooldown=%u term=%u\r\n",
               (unsigned int)frame->data[0],
               (unsigned int)frame->data[1],
               (unsigned int)frame->data[2],
               (unsigned int)frame->data[3]);
        return;
    }

    if(frame->id == CAN_ID_GLOBAL_STATE)
    {
        NodeRole_OnGlobalState(frame->data[0],
                               frame->data[1],
                               frame->data[2]);
        GameMaster_OnGlobalState(frame);
        GamePlayer_OnGlobalState(frame);
#if NODE_ID == NODE_ID_BOARD_A
        GameStatusPanel_SetHealth(frame->data[3], frame->data[4]);
        GameStatusPanel_SetGameState(frame->data[7]);
#endif
        return;
    }

    if(frame->id == CAN_ID_POSITION_STATE)
    {
        GameMaster_OnPositionState(frame);
        return;
    }

    if(frame->id == CAN_ID_CRYSTAL_ATTACK)
    {
        /* Master already applied the hit; other nodes consume 0x100 state. */
        return;
    }

    if((frame->id >= CAN_ID_HEARTBEAT_BASE) &&
       (frame->id < (CAN_ID_HEARTBEAT_BASE + NODE_COUNT)))
    {
        node_id = frame->data[0];
        if(Protocol_IdMatchesNode(frame->id,
                                  CAN_ID_HEARTBEAT_BASE,
                                  node_id))
        {
            NodeRole_OnHeartbeat(node_id,
                                 frame->data[1],
                                 frame->data[2],
                                 frame->data[3],
                                 frame->data[4],
                                 frame->data[5]);
        }
        return;
    }

    if((frame->id >= CAN_ID_MASTER_CLAIM_BASE) &&
       (frame->id < (CAN_ID_MASTER_CLAIM_BASE + NODE_COUNT)))
    {
        node_id = frame->data[0];
        if(Protocol_IdMatchesNode(frame->id,
                                  CAN_ID_MASTER_CLAIM_BASE,
                                  node_id))
        {
            NodeRole_OnMasterClaim(node_id,
                                   frame->data[1],
                                   frame->data[2],
                                   frame->data[3]);
        }
        return;
    }

    if((frame->id >= CAN_ID_SKILL_INPUT_BASE) &&
       (frame->id < (CAN_ID_SKILL_INPUT_BASE + NODE_COUNT)))
    {
        node_id = frame->data[0];
        if(!Protocol_IdMatchesNode(frame->id,
                                   CAN_ID_SKILL_INPUT_BASE,
                                   node_id) ||
           !GameCommon_IsValidSkill(frame->data[1]))
        {
            return;
        }

        if(node_id == NODE_ID_PC)
        {
            GameMaster_OnPcSkill(frame->data[1], frame->data[5]);
        }
        else
        {
            /*
             * Master+Player 已在发送路径本地处理，避免可能的自回环重复结算。
             */
            if(!((node_id == NODE_ID) &&
                 NodeRole_IsMaster() &&
                 NodeRole_IsPlayer()))
            {
                GameMaster_OnEmbeddedSkill(node_id,
                                           frame->data[1],
                                           frame->data[5]);
            }
        }
        return;
    }

    if((frame->id >= CAN_ID_SKILL_RESULT_BASE) &&
       (frame->id < (CAN_ID_SKILL_RESULT_BASE + NODE_COUNT)))
    {
        /* 技能结果供 PC 上位机确认，板端无需重复结算。 */
        return;
    }

    if((frame->id >= CAN_ID_MOVE_INPUT_BASE) &&
       (frame->id < (CAN_ID_MOVE_INPUT_BASE + NODE_COUNT)))
    {
        node_id = frame->data[0];
        if(Protocol_IdMatchesNode(frame->id,
                                  CAN_ID_MOVE_INPUT_BASE,
                                  node_id))
        {
            if(!((node_id == NODE_ID) &&
                 NodeRole_IsMaster() &&
                 NodeRole_IsPlayer()))
            {
                GameMaster_OnMoveInput(node_id,
                                       frame->data[1],
                                       frame->data[2]);
            }
        }
        return;
    }

    if((frame->id >= CAN_ID_READY_BASE) &&
       (frame->id < (CAN_ID_READY_BASE + NODE_COUNT)))
    {
        node_id = frame->data[0];
        if(Protocol_IdMatchesNode(frame->id,
                                  CAN_ID_READY_BASE,
                                  node_id) &&
           (frame->data[1] == 1U))
        {
            NodeRole_OnReady(node_id, frame->data[2]);
        }
        return;
    }

    if((frame->id >= CAN_ID_HERO_STATE_BASE) &&
       (frame->id < (CAN_ID_HERO_STATE_BASE + NODE_COUNT)))
    {
        return;
    }
}
