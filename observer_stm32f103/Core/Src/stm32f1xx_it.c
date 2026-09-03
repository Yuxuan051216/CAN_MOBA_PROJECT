#include "stm32f1xx_it.h"

#include "main.h"
#include "rgb_effect.h"

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void DMA1_Channel1_IRQHandler(void)
{
    RgbEffect_DmaIrqHandler();
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
    HAL_CAN_IRQHandler(&hcan);
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}
