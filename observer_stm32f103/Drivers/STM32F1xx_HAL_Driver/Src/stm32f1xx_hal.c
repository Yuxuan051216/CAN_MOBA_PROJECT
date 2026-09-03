#include "stm32f1xx_hal.h"

static volatile uint32_t hal_tick;

HAL_StatusTypeDef HAL_Init(void)
{
    hal_tick = 0U;
    if(SysTick_Config(SystemCoreClock / 1000UL) != 0U)
    {
        return HAL_ERROR;
    }
    return HAL_OK;
}

uint32_t HAL_GetTick(void)
{
    return hal_tick;
}

void HAL_IncTick(void)
{
    hal_tick++;
}

void HAL_Delay(uint32_t delay_ms)
{
    uint32_t start = HAL_GetTick();
    while((uint32_t)(HAL_GetTick() - start) < delay_ms)
    {
        __WFI();
    }
}

void HAL_NVIC_SetPriority(IRQn_Type irqn,
                          uint32_t preempt_priority,
                          uint32_t sub_priority)
{
    (void)sub_priority;
    NVIC_SetPriority(irqn, preempt_priority);
}

void HAL_NVIC_EnableIRQ(IRQn_Type irqn)
{
    NVIC_EnableIRQ(irqn);
}

static uint32_t HAL_GPIO_ConfigNibble(const GPIO_InitTypeDef *init)
{
    uint32_t speed = init->Speed;

    if(speed > GPIO_SPEED_FREQ_HIGH)
    {
        speed = GPIO_SPEED_FREQ_HIGH;
    }

    switch(init->Mode)
    {
        case GPIO_MODE_OUTPUT_PP:
            return speed;
        case GPIO_MODE_OUTPUT_OD:
            return speed | 0x4U;
        case GPIO_MODE_AF_PP:
            return speed | 0x8U;
        case GPIO_MODE_AF_OD:
            return speed | 0xCU;
        case GPIO_MODE_INPUT_PULLUP:
            return 0x8U;
        case GPIO_MODE_INPUT_FLOATING:
            return 0x4U;
        default:
            return 0U;
    }
}

void HAL_GPIO_Init(GPIO_TypeDef *gpio, const GPIO_InitTypeDef *init)
{
    uint32_t pin;
    uint32_t config = HAL_GPIO_ConfigNibble(init);

    if((gpio == 0) || (init == 0))
    {
        return;
    }

    for(pin = 0U; pin < 16U; pin++)
    {
        uint32_t mask = 1UL << pin;
        uint32_t shift = (pin & 7U) * 4U;
        volatile uint32_t *cr;

        if((init->Pin & mask) == 0U)
        {
            continue;
        }

        cr = (pin < 8U) ? &gpio->CRL : &gpio->CRH;
        *cr = (*cr & ~(0xFUL << shift)) | (config << shift);
        if(init->Mode == GPIO_MODE_INPUT_PULLUP)
        {
            if(init->Pull == GPIO_PULLDOWN)
            {
                gpio->BRR = mask;
            }
            else
            {
                gpio->BSRR = mask;
            }
        }
    }
}

void HAL_GPIO_WritePin(GPIO_TypeDef *gpio,
                       uint16_t pin,
                       GPIO_PinState state)
{
    if(state == GPIO_PIN_SET)
    {
        gpio->BSRR = pin;
    }
    else
    {
        gpio->BRR = pin;
    }
}

void HAL_GPIO_TogglePin(GPIO_TypeDef *gpio, uint16_t pin)
{
    if((gpio->ODR & pin) != 0U)
    {
        gpio->BRR = pin;
    }
    else
    {
        gpio->BSRR = pin;
    }
}

HAL_StatusTypeDef HAL_CAN_Init(CAN_HandleTypeDef *hcan)
{
    uint32_t timeout;
    uint32_t btr;

    if((hcan == 0) || (hcan->Instance == 0) ||
       (hcan->Init.Prescaler == 0U))
    {
        return HAL_ERROR;
    }

    hcan->Instance->MCR |= 1UL;
    timeout = 1000000UL;
    while(((hcan->Instance->MSR & 1UL) == 0U) && (timeout-- != 0U))
    {
    }
    if(timeout == 0U)
    {
        return HAL_TIMEOUT;
    }

    hcan->Instance->MCR = 1UL;
    if(hcan->Init.AutoBusOff != 0U)
    {
        hcan->Instance->MCR |= (1UL << 6);
    }
    if(hcan->Init.AutoWakeUp != 0U)
    {
        hcan->Instance->MCR |= (1UL << 5);
    }
    if(hcan->Init.AutoRetransmission == 0U)
    {
        hcan->Instance->MCR |= (1UL << 4);
    }
    if(hcan->Init.ReceiveFifoLocked != 0U)
    {
        hcan->Instance->MCR |= (1UL << 3);
    }
    if(hcan->Init.TransmitFifoPriority != 0U)
    {
        hcan->Instance->MCR |= (1UL << 2);
    }

    btr = ((hcan->Init.SyncJumpWidth & 0x3U) << 24) |
          ((hcan->Init.TimeSeg2 & 0x7U) << 20) |
          ((hcan->Init.TimeSeg1 & 0xFU) << 16) |
          ((hcan->Init.Prescaler - 1U) & 0x3FFU);
    hcan->Instance->BTR = btr;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_ConfigFilter(
    CAN_HandleTypeDef *hcan,
    const CAN_FilterTypeDef *filter)
{
    uint32_t bank_bit;

    if((hcan == 0) || (filter == 0) || (filter->FilterBank >= 14U))
    {
        return HAL_ERROR;
    }

    bank_bit = 1UL << filter->FilterBank;
    hcan->Instance->FMR |= 1UL;
    hcan->Instance->FA1R &= ~bank_bit;

    if(filter->FilterScale == CAN_FILTERSCALE_16BIT)
    {
        hcan->Instance->FS1R &= ~bank_bit;
    }
    else
    {
        hcan->Instance->FS1R |= bank_bit;
    }

    if(filter->FilterMode == CAN_FILTERMODE_IDLIST)
    {
        hcan->Instance->FM1R |= bank_bit;
    }
    else
    {
        hcan->Instance->FM1R &= ~bank_bit;
    }

    if(filter->FilterFIFOAssignment == CAN_FILTER_FIFO0)
    {
        hcan->Instance->FFA1R &= ~bank_bit;
    }
    else
    {
        hcan->Instance->FFA1R |= bank_bit;
    }

    hcan->Instance->sFilterRegister[filter->FilterBank].FR1 =
        (filter->FilterIdHigh << 16) |
        (filter->FilterIdLow & 0xFFFFU);
    hcan->Instance->sFilterRegister[filter->FilterBank].FR2 =
        (filter->FilterMaskIdHigh << 16) |
        (filter->FilterMaskIdLow & 0xFFFFU);

    if(filter->FilterActivation != 0U)
    {
        hcan->Instance->FA1R |= bank_bit;
    }
    hcan->Instance->FMR &= ~1UL;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
    uint32_t timeout;

    if((hcan == 0) || (hcan->Instance == 0))
    {
        return HAL_ERROR;
    }

    hcan->Instance->MCR &= ~1UL;
    timeout = 1000000UL;
    while(((hcan->Instance->MSR & 1UL) != 0U) && (timeout-- != 0U))
    {
    }
    return (timeout == 0U) ? HAL_TIMEOUT : HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(
    CAN_HandleTypeDef *hcan,
    uint32_t notifications)
{
    if((hcan == 0) || (hcan->Instance == 0))
    {
        return HAL_ERROR;
    }
    hcan->Instance->IER |= notifications;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_CAN_GetRxMessage(
    CAN_HandleTypeDef *hcan,
    uint32_t fifo,
    CAN_RxHeaderTypeDef *header,
    uint8_t data[8])
{
    uint32_t rir;
    uint32_t rdtr;
    uint32_t low;
    uint32_t high;
    uint32_t i;

    if((hcan == 0) || (header == 0) || (data == 0) ||
       (fifo != CAN_RX_FIFO0) ||
       ((hcan->Instance->RF0R & 0x3U) == 0U))
    {
        return HAL_ERROR;
    }

    rir = hcan->Instance->sFIFOMailBox[0].RIR;
    rdtr = hcan->Instance->sFIFOMailBox[0].RDTR;
    low = hcan->Instance->sFIFOMailBox[0].RDLR;
    high = hcan->Instance->sFIFOMailBox[0].RDHR;

    header->IDE = rir & CAN_ID_EXT;
    header->RTR = rir & 0x2U;
    header->StdId = (rir >> 21) & 0x7FFU;
    header->ExtId = (rir >> 3) & 0x1FFFFFFFU;
    header->DLC = rdtr & 0xFU;
    header->FilterMatchIndex = (rdtr >> 8) & 0xFFU;
    header->Timestamp = (rdtr >> 16) & 0xFFFFU;

    for(i = 0U; i < 4U; i++)
    {
        data[i] = (uint8_t)(low >> (8U * i));
        data[i + 4U] = (uint8_t)(high >> (8U * i));
    }

    hcan->Instance->RF0R |= (1UL << 5);
    return HAL_OK;
}

void HAL_CAN_IRQHandler(CAN_HandleTypeDef *hcan)
{
    if((hcan != 0) &&
       ((hcan->Instance->IER & CAN_IT_RX_FIFO0_MSG_PENDING) != 0U) &&
       ((hcan->Instance->RF0R & 0x3U) != 0U))
    {
        HAL_CAN_RxFifo0MsgPendingCallback(hcan);
    }
}

__weak void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
}

HAL_StatusTypeDef HAL_UART_Init(UART_HandleTypeDef *huart)
{
    uint32_t peripheral_clock;
    uint32_t divider;

    if((huart == 0) || (huart->Instance == 0) ||
       (huart->Init.BaudRate == 0U))
    {
        return HAL_ERROR;
    }

    peripheral_clock =
        (huart->Instance == USART1) ? 72000000UL : 36000000UL;
    divider = (peripheral_clock + (huart->Init.BaudRate / 2U)) /
              huart->Init.BaudRate;
    huart->Instance->CR1 = 0U;
    huart->Instance->CR2 = 0U;
    huart->Instance->CR3 = 0U;
    huart->Instance->BRR = divider;
    huart->Instance->CR1 = USART_CR1_UE |
                           USART_CR1_TE |
                           USART_CR1_RE;
    huart->gState = HAL_UART_STATE_READY;
    huart->TxXferCount = 0U;
    return HAL_OK;
}

HAL_StatusTypeDef HAL_UART_Transmit_IT(UART_HandleTypeDef *huart,
                                       const uint8_t *data,
                                       uint16_t size)
{
    if((huart == 0) || (data == 0) || (size == 0U))
    {
        return HAL_ERROR;
    }
    if(huart->gState != HAL_UART_STATE_READY)
    {
        return HAL_BUSY;
    }

    huart->pTxBuffPtr = data;
    huart->TxXferSize = size;
    huart->TxXferCount = size;
    huart->gState = HAL_UART_STATE_BUSY_TX;
    huart->Instance->CR1 |= USART_CR1_TXEIE;
    return HAL_OK;
}

void HAL_UART_IRQHandler(UART_HandleTypeDef *huart)
{
    uint32_t status;

    if(huart == 0)
    {
        return;
    }

    status = huart->Instance->SR;
    if(((status & USART_SR_TXE) != 0U) &&
       ((huart->Instance->CR1 & USART_CR1_TXEIE) != 0U))
    {
        if(huart->TxXferCount != 0U)
        {
            huart->Instance->DR = *huart->pTxBuffPtr++;
            huart->TxXferCount--;
        }
        if(huart->TxXferCount == 0U)
        {
            huart->Instance->CR1 &= ~USART_CR1_TXEIE;
            huart->Instance->CR1 |= USART_CR1_TCIE;
        }
    }

    if(((status & USART_SR_TC) != 0U) &&
       ((huart->Instance->CR1 & USART_CR1_TCIE) != 0U))
    {
        huart->Instance->CR1 &= ~USART_CR1_TCIE;
        huart->gState = HAL_UART_STATE_READY;
        HAL_UART_TxCpltCallback(huart);
    }
}

__weak void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    (void)huart;
}
