#include "can_moba_protocol.h"

uint8_t ObserverProtocol_IsObservedId(uint16_t id)
{
    return (uint8_t)((id == CAN_ID_ROLE_SWITCH) ||
                     (id == CAN_ID_GAME_CTRL) ||
                     (id == CAN_ID_DEATH_EVENT) ||
                     (id == CAN_ID_GLOBAL_STATE) ||
                     (id == CAN_ID_POSITION_STATE) ||
                     (id == CAN_ID_CRYSTAL_ATTACK) ||
                     (id == CAN_ID_SKILL_RESULT_BOARD_A) ||
                     (id == CAN_ID_SKILL_RESULT_BOARD_B));
}

uint8_t ObserverProtocol_IsBoardNode(uint8_t node_id)
{
    return (uint8_t)((node_id == NODE_ID_BOARD_A) ||
                     (node_id == NODE_ID_BOARD_B));
}
