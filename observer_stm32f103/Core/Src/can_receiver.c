#include "can_receiver.h"

#include "can_moba_protocol.h"
#include "event_queue.h"

static uint32_t rejected_count;

static uint16_t CanReceiver_FilterValue(uint16_t id)
{
    return (uint16_t)(id << 5);
}

static HAL_StatusTypeDef CanReceiver_AddFilter(
    CAN_HandleTypeDef *hcan,
    uint32_t bank,
    uint16_t id0,
    uint16_t id1,
    uint16_t id2,
    uint16_t id3)
{
    CAN_FilterTypeDef filter = {0};

    filter.FilterBank = bank;
    filter.FilterMode = CAN_FILTERMODE_IDLIST;
    filter.FilterScale = CAN_FILTERSCALE_16BIT;
    filter.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    filter.FilterActivation = ENABLE;
    filter.SlaveStartFilterBank = 14U;
    filter.FilterIdHigh = CanReceiver_FilterValue(id0);
    filter.FilterIdLow = CanReceiver_FilterValue(id1);
    filter.FilterMaskIdHigh = CanReceiver_FilterValue(id2);
    filter.FilterMaskIdLow = CanReceiver_FilterValue(id3);
    return HAL_CAN_ConfigFilter(hcan, &filter);
}

HAL_StatusTypeDef CanReceiver_Init(CAN_HandleTypeDef *hcan)
{
    rejected_count = 0U;
    if(CanReceiver_AddFilter(hcan, 0U,
                             CAN_ID_ROLE_SWITCH,
                             CAN_ID_GAME_CTRL,
                             CAN_ID_DEATH_EVENT,
                             CAN_ID_GLOBAL_STATE) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if(CanReceiver_AddFilter(hcan, 1U,
                             CAN_ID_POSITION_STATE,
                             CAN_ID_CRYSTAL_ATTACK,
                             CAN_ID_SKILL_RESULT_BOARD_A,
                             CAN_ID_SKILL_RESULT_BOARD_B) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if(HAL_CAN_Start(hcan) != HAL_OK)
    {
        return HAL_ERROR;
    }
    if(HAL_CAN_ActivateNotification(
           hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        return HAL_ERROR;
    }
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 1U, 0U);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
    return HAL_OK;
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef header;
    ObserverCanFrame frame;

    while((hcan->Instance->RF0R & 0x3U) != 0U)
    {
        if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0,
                               &header, frame.data) != HAL_OK)
        {
            rejected_count++;
            return;
        }
        if((header.IDE != CAN_ID_STD) ||
           (header.RTR != CAN_RTR_DATA) ||
           (header.DLC != 8U) ||
           !ObserverProtocol_IsObservedId((uint16_t)header.StdId))
        {
            rejected_count++;
            continue;
        }

        frame.id = (uint16_t)header.StdId;
        frame.dlc = (uint8_t)header.DLC;
        (void)EventQueue_PushFromIsr(&frame);
    }
}

uint32_t CanReceiver_GetRejectedCount(void)
{
    return rejected_count;
}
