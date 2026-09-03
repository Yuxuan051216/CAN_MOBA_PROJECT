#include "node_role.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_time.h"
#include "game_common.h"
#include "game_protocol.h"

NodeRoleContext_t g_node_role;

static void NodeRole_RefreshSelfRole(void)
{
    if(g_node_role.is_master && g_node_role.is_player)
    {
        g_node_role.self_role = ROLE_MASTER_PLAYER;
    }
    else if(g_node_role.is_master)
    {
        g_node_role.self_role = ROLE_MASTER;
    }
    else if(g_node_role.is_player)
    {
        g_node_role.self_role = ROLE_PLAYER;
    }
    else if(g_node_role.self_hero_state == HERO_COOLDOWN)
    {
        g_node_role.self_role = ROLE_COOLDOWN;
    }
    else
    {
        g_node_role.self_role = ROLE_BACKUP;
    }
}

static void NodeRole_SetOnline(uint8_t node_id, uint8_t online)
{
    if(node_id == NODE_ID_PC)
    {
        g_node_role.pc_online = online;
    }
    else if(node_id == NODE_ID_BOARD_A)
    {
        g_node_role.board_a_online = online;
    }
    else if(node_id == NODE_ID_BOARD_B)
    {
        g_node_role.board_b_online = online;
    }
}

void NodeRole_Init(void)
{
    uint32_t now = millis();
    uint8_t i;

    g_node_role.self_node_id = NODE_ID;
    g_node_role.self_hero_state = HERO_ALIVE;
    g_node_role.current_master = NODE_ID_BOARD_A;
    g_node_role.current_player = NODE_ID_BOARD_B;
    g_node_role.current_term = 1U;
    g_node_role.last_master_heartbeat_ms = now;

    for(i = 0U; i < NODE_COUNT; i++)
    {
        g_node_role.last_node_heartbeat_ms[i] = 0U;
        g_node_role.node_hero_state[i] = HERO_READY;
    }
    g_node_role.last_node_heartbeat_ms[NODE_ID] = now;
    g_node_role.node_hero_state[NODE_ID] = HERO_ALIVE;

    g_node_role.board_a_online = (uint8_t)(NODE_ID == NODE_ID_BOARD_A);
    g_node_role.board_b_online = (uint8_t)(NODE_ID == NODE_ID_BOARD_B);
    g_node_role.pc_online = 0U;

    g_node_role.cooldown_remaining_s = 0U;
    g_node_role.cooldown_end_ms = 0U;
    g_node_role.is_master = (uint8_t)(NODE_ID == NODE_ID_BOARD_A);
    g_node_role.is_player = (uint8_t)(NODE_ID == NODE_ID_BOARD_B);
    g_node_role.election_active = 0U;
    g_node_role.election_term = 0U;
    g_node_role.election_candidate = 0xFFU;
    g_node_role.election_priority = 0xFFU;
    g_node_role.ready_sent = 0U;
    g_node_role.election_deadline_ms = 0U;
    g_node_role.last_claim_ms = 0U;

    NodeRole_RefreshSelfRole();
}

void NodeRole_OnHeartbeat(uint8_t node_id,
                          uint8_t role,
                          uint8_t hero_state,
                          uint8_t hp,
                          uint8_t term,
                          uint8_t cooldown_s)
{
    uint32_t now;

    (void)hp;
    (void)cooldown_s;

    if(!GameCommon_IsValidNode(node_id))
    {
        return;
    }

    now = millis();
    g_node_role.last_node_heartbeat_ms[node_id] = now;
    g_node_role.node_hero_state[node_id] =
        (HeroState_t)hero_state;
    NodeRole_SetOnline(node_id, 1U);

    if(node_id == g_node_role.current_master)
    {
        g_node_role.last_master_heartbeat_ms = now;
        g_node_role.election_active = 0U;
    }

    if((term > g_node_role.current_term) &&
       ((role == ROLE_MASTER) || (role == ROLE_MASTER_PLAYER)))
    {
        g_node_role.current_term = term;
        g_node_role.current_master = node_id;
        g_node_role.is_master = (uint8_t)(NODE_ID == node_id);
        g_node_role.last_master_heartbeat_ms = now;
        NodeRole_RefreshSelfRole();
    }
}

void NodeRole_OnRoleSwitch(uint8_t term,
                           uint8_t new_master,
                           uint8_t new_player,
                           uint8_t reason,
                           uint8_t target_node,
                           uint8_t cooldown_s)
{
    uint32_t now = millis();

    if((term < g_node_role.current_term) ||
       ((term == g_node_role.current_term) &&
        ((new_master != g_node_role.current_master) ||
         (new_player != g_node_role.current_player))) ||
       !GameCommon_IsValidNode(new_master) ||
       !GameCommon_IsValidNode(new_player) ||
       (new_master == NODE_ID_PC))
    {
        return;
    }

    g_node_role.current_term = term;
    g_node_role.current_master = new_master;
    g_node_role.current_player = new_player;
    g_node_role.is_master = (uint8_t)(NODE_ID == new_master);
    g_node_role.is_player = (uint8_t)(NODE_ID == new_player);
    g_node_role.last_master_heartbeat_ms = now;
    g_node_role.election_active = 0U;

    if(reason == SWITCH_BY_MANUAL_RESET)
    {
        g_node_role.self_hero_state = HERO_ALIVE;
        g_node_role.node_hero_state[NODE_ID_BOARD_A] = HERO_ALIVE;
        g_node_role.node_hero_state[NODE_ID_BOARD_B] = HERO_ALIVE;
        g_node_role.cooldown_remaining_s = 0U;
        g_node_role.cooldown_end_ms = 0U;
        g_node_role.ready_sent = 0U;
        g_node_role.node_hero_state[NODE_ID] = HERO_ALIVE;
    }
    else if((reason == SWITCH_BY_PLAYER_DEATH) && (NODE_ID == target_node))
    {
        g_node_role.self_hero_state = HERO_COOLDOWN;
        g_node_role.cooldown_remaining_s = cooldown_s;
        g_node_role.cooldown_end_ms = now + ((uint32_t)cooldown_s * 1000U);
        g_node_role.ready_sent = 0U;
        g_node_role.node_hero_state[NODE_ID] = HERO_COOLDOWN;
    }
    else if(g_node_role.is_player)
    {
        g_node_role.self_hero_state = HERO_ALIVE;
        g_node_role.cooldown_remaining_s = 0U;
        g_node_role.cooldown_end_ms = 0U;
        g_node_role.ready_sent = 0U;
        g_node_role.node_hero_state[NODE_ID] = HERO_ALIVE;
    }

    NodeRole_RefreshSelfRole();

    printf("Role switch: term=%u master=%u player=%u reason=%u target=%u\r\n",
           (unsigned int)term,
           (unsigned int)new_master,
           (unsigned int)new_player,
           (unsigned int)reason,
           (unsigned int)target_node);
}

void NodeRole_OnMasterClaim(uint8_t node_id,
                            uint8_t requested_term,
                            uint8_t priority,
                            uint8_t hero_state)
{
    uint32_t now;

    (void)hero_state;

    if((node_id == NODE_ID_PC) || !GameCommon_IsValidNode(node_id) ||
       (requested_term < (uint8_t)(g_node_role.current_term + 1U)))
    {
        return;
    }

    now = millis();

    if((g_node_role.election_active == 0U) ||
       (requested_term > g_node_role.election_term))
    {
        g_node_role.election_active = 1U;
        g_node_role.election_term = requested_term;
        g_node_role.election_candidate = node_id;
        g_node_role.election_priority = priority;
        g_node_role.election_deadline_ms = now + MASTER_CLAIM_WINDOW_MS;
    }

    if((requested_term == g_node_role.election_term) &&
       ((priority < g_node_role.election_priority) ||
        ((priority == g_node_role.election_priority) &&
         (node_id < g_node_role.election_candidate))))
    {
        g_node_role.election_candidate = node_id;
        g_node_role.election_priority = priority;
    }
}

void NodeRole_OnGlobalState(uint8_t term,
                            uint8_t master_node,
                            uint8_t player_node)
{
    if((term < g_node_role.current_term) ||
       ((term == g_node_role.current_term) &&
        (master_node != g_node_role.current_master)) ||
       (master_node == NODE_ID_PC) ||
       !GameCommon_IsValidNode(master_node) ||
       !GameCommon_IsValidNode(player_node))
    {
        return;
    }

    g_node_role.current_term = term;
    g_node_role.current_master = master_node;
    g_node_role.current_player = player_node;
    g_node_role.is_master = (uint8_t)(NODE_ID == master_node);
    g_node_role.is_player = (uint8_t)(NODE_ID == player_node);
    g_node_role.last_master_heartbeat_ms = millis();
    g_node_role.election_active = 0U;
    NodeRole_RefreshSelfRole();
}

void NodeRole_OnReady(uint8_t node_id, uint8_t term)
{
    if((node_id == NODE_ID) && (term >= g_node_role.current_term))
    {
        g_node_role.self_hero_state = HERO_READY;
        g_node_role.node_hero_state[NODE_ID] = HERO_READY;
        g_node_role.cooldown_remaining_s = 0U;
        NodeRole_RefreshSelfRole();
    }
}

void NodeRole_OnDeathEvent(uint8_t dead_node, uint8_t cooldown_s)
{
    uint32_t now;

    if((dead_node != NODE_ID) || (cooldown_s == 0U))
    {
        return;
    }

    now = millis();
    g_node_role.self_hero_state = HERO_COOLDOWN;
    g_node_role.node_hero_state[NODE_ID] = HERO_COOLDOWN;
    g_node_role.cooldown_remaining_s = cooldown_s;
    g_node_role.cooldown_end_ms = now + ((uint32_t)cooldown_s * 1000U);
    g_node_role.ready_sent = 0U;
    NodeRole_RefreshSelfRole();
}

void NodeRole_OnMasterTimeout(void)
{
    uint32_t now;

    if(g_node_role.is_master || g_node_role.election_active)
    {
        return;
    }

    now = millis();
    g_node_role.election_active = 1U;
    g_node_role.election_term = (uint8_t)(g_node_role.current_term + 1U);
    g_node_role.election_candidate = NODE_ID;
    g_node_role.election_priority = NODE_ID;
    g_node_role.election_deadline_ms = now + MASTER_CLAIM_WINDOW_MS;
    g_node_role.last_claim_ms = now;

    printf("Master timeout: claim term=%u node=%u\r\n",
           (unsigned int)g_node_role.election_term,
           (unsigned int)NODE_ID);
    Protocol_SendMasterClaim();
}

void NodeRole_Update(void)
{
    uint32_t now = millis();
    uint8_t node_id;
    uint8_t old_master;
    uint8_t new_player;
    uint8_t recovered_player;
    uint8_t new_term;

    g_node_role.last_node_heartbeat_ms[NODE_ID] = now;
    NodeRole_SetOnline(NODE_ID, 1U);

    for(node_id = 0U; node_id < NODE_COUNT; node_id++)
    {
        if(node_id == NODE_ID)
        {
            continue;
        }

        if((g_node_role.last_node_heartbeat_ms[node_id] == 0U) ||
           ((uint32_t)(now - g_node_role.last_node_heartbeat_ms[node_id]) >
            NODE_ONLINE_TIMEOUT_MS))
        {
            NodeRole_SetOnline(node_id, 0U);
        }
    }

    if(g_node_role.self_hero_state == HERO_COOLDOWN)
    {
        g_node_role.cooldown_remaining_s =
            GameCommon_RemainingSeconds(g_node_role.cooldown_end_ms, now);

        if((g_node_role.cooldown_remaining_s == 0U) &&
           (g_node_role.ready_sent == 0U))
        {
            g_node_role.self_hero_state = HERO_READY;
            g_node_role.node_hero_state[NODE_ID] = HERO_READY;
            g_node_role.ready_sent = 1U;
            NodeRole_RefreshSelfRole();
            Protocol_SendReady();
        }
    }

    if(g_node_role.is_master)
    {
        /*
         * 单板掉线时允许 Master 兼任 Player；另一块板恢复在线后，
         * 当前 Master 留在场下，恢复节点重新接管 Player。
         */
        if(g_node_role.current_master == g_node_role.current_player)
        {
            recovered_player =
                (g_node_role.current_master == NODE_ID_BOARD_A) ?
                NODE_ID_BOARD_B : NODE_ID_BOARD_A;

            if(NodeRole_IsNodeOnline(recovered_player))
            {
                new_term = (uint8_t)(g_node_role.current_term + 1U);
                NodeRole_OnRoleSwitch(new_term,
                                      g_node_role.current_master,
                                      recovered_player,
                                      SWITCH_BY_NODE_RECOVERY,
                                      recovered_player,
                                      0U);
                Protocol_SendRoleSwitch(g_node_role.current_master,
                                        recovered_player,
                                        SWITCH_BY_NODE_RECOVERY,
                                        recovered_player,
                                        0U);
                Protocol_SendGlobalState();
                printf("Role recovery: Master=%u Player=%u\r\n",
                       (unsigned int)g_node_role.current_master,
                       (unsigned int)recovered_player);
            }
        }

        g_node_role.last_master_heartbeat_ms = now;
        g_node_role.election_active = 0U;
        return;
    }

    if((uint32_t)(now - g_node_role.last_master_heartbeat_ms) >
       MASTER_TIMEOUT_MS)
    {
        NodeRole_OnMasterTimeout();
    }

    if(g_node_role.election_active &&
       GameCommon_TimeReached(now, g_node_role.election_deadline_ms))
    {
        if(g_node_role.election_candidate == NODE_ID)
        {
            old_master = g_node_role.current_master;
            new_player = g_node_role.current_player;

            if((new_player == old_master) ||
               !NodeRole_IsNodeOnline(new_player))
            {
                new_player = NODE_ID;
            }

            NodeRole_OnRoleSwitch(g_node_role.election_term,
                                  NODE_ID,
                                  new_player,
                                  SWITCH_BY_MASTER_TIMEOUT,
                                  old_master,
                                  0U);
            Protocol_SendRoleSwitch(NODE_ID,
                                    new_player,
                                    SWITCH_BY_MASTER_TIMEOUT,
                                    old_master,
                                    0U);
        }
        else
        {
            g_node_role.current_master = g_node_role.election_candidate;
            g_node_role.last_master_heartbeat_ms = now;
            g_node_role.election_active = 0U;
        }
    }
}

uint8_t NodeRole_IsMaster(void)
{
    return g_node_role.is_master;
}

uint8_t NodeRole_IsPlayer(void)
{
    return g_node_role.is_player;
}

uint8_t NodeRole_IsCooldown(void)
{
    return (uint8_t)(g_node_role.self_hero_state == HERO_COOLDOWN);
}

uint8_t NodeRole_IsNodeOnline(uint8_t node_id)
{
    if(node_id == NODE_ID_PC)
    {
        return g_node_role.pc_online;
    }
    if(node_id == NODE_ID_BOARD_A)
    {
        return g_node_role.board_a_online;
    }
    if(node_id == NODE_ID_BOARD_B)
    {
        return g_node_role.board_b_online;
    }
    return 0U;
}

HeroState_t NodeRole_GetNodeHeroState(uint8_t node_id)
{
    if(node_id >= NODE_COUNT)
    {
        return HERO_DEAD;
    }
    return g_node_role.node_hero_state[node_id];
}

NodeRole_t NodeRole_GetHeartbeatRole(void)
{
    return g_node_role.self_role;
}

void NodeRole_SetHeroState(HeroState_t state)
{
    g_node_role.self_hero_state = state;
    g_node_role.node_hero_state[NODE_ID] = state;
    NodeRole_RefreshSelfRole();
}
