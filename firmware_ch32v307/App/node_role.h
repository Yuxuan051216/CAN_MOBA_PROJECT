#ifndef NODE_ROLE_H
#define NODE_ROLE_H

#include <stdint.h>

#include "game_types.h"

typedef struct {
    uint8_t self_node_id;
    NodeRole_t self_role;
    HeroState_t self_hero_state;

    uint8_t current_master;
    uint8_t current_player;
    uint8_t current_term;

    uint32_t last_master_heartbeat_ms;
    uint32_t last_node_heartbeat_ms[3];
    HeroState_t node_hero_state[3];

    uint8_t board_a_online;
    uint8_t board_b_online;
    uint8_t pc_online;

    uint8_t cooldown_remaining_s;
    uint32_t cooldown_end_ms;

    uint8_t is_master;
    uint8_t is_player;
    uint8_t election_active;
    uint8_t election_term;
    uint8_t election_candidate;
    uint8_t election_priority;
    uint8_t ready_sent;
    uint32_t election_deadline_ms;
    uint32_t last_claim_ms;
} NodeRoleContext_t;

extern NodeRoleContext_t g_node_role;

void NodeRole_Init(void);
void NodeRole_Update(void);
void NodeRole_OnHeartbeat(uint8_t node_id,
                          uint8_t role,
                          uint8_t hero_state,
                          uint8_t hp,
                          uint8_t term,
                          uint8_t cooldown_s);
void NodeRole_OnRoleSwitch(uint8_t term,
                           uint8_t new_master,
                           uint8_t new_player,
                           uint8_t reason,
                           uint8_t target_node,
                           uint8_t cooldown_s);
void NodeRole_OnMasterClaim(uint8_t node_id,
                            uint8_t requested_term,
                            uint8_t priority,
                            uint8_t hero_state);
void NodeRole_OnGlobalState(uint8_t term,
                            uint8_t master_node,
                            uint8_t player_node);
void NodeRole_OnReady(uint8_t node_id, uint8_t term);
void NodeRole_OnDeathEvent(uint8_t dead_node, uint8_t cooldown_s);
void NodeRole_OnMasterTimeout(void);

uint8_t NodeRole_IsMaster(void);
uint8_t NodeRole_IsPlayer(void);
uint8_t NodeRole_IsCooldown(void);
uint8_t NodeRole_IsNodeOnline(uint8_t node_id);
HeroState_t NodeRole_GetNodeHeroState(uint8_t node_id);
NodeRole_t NodeRole_GetHeartbeatRole(void);
void NodeRole_SetHeroState(HeroState_t state);

#endif
