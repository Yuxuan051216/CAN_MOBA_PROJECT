/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : Main Interrupt Service Routines.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#include "dmx_all.h"
#include "bsp_time.h"

// 定时器中断服务函数
void pit1_irq(void)
{

}

void pit2_irq(void)
{
    BSP_Time_On1msInterrupt();
}

void pit3_irq(void)
{

}

void pit4_irq(void)
{

}

void pit5_irq(void)
{

}

void pit6_irq(void)
{

}

void pit7_irq(void)
{

}

void pit8_irq(void)
{

}

void pit9_irq(void)
{

}

void pit10_irq(void)
{

}

// 外部中断服务函数
void exti0_irq(void)
{

}

void exti1_irq(void)
{

}

void exti2_irq(void)
{

}

void exti3_irq(void)
{

}

void exti4_irq(void)
{

}

void exti5_irq(void)
{

}

void exti6_irq(void)
{

}

void exti7_irq(void)
{

}

void exti8_irq(void)
{

}

void exti9_irq(void)
{

}

void exti10_irq(void)
{

}

void exti11_irq(void)
{

}

void exti12_irq(void)
{

}

void exti13_irq(void)
{

}

void exti14_irq(void)
{

}

void exti15_irq(void)
{

}

// 串口接收中断函数
void usart1_rx_irq(void)
{

}

void usart2_rx_irq(void)
{

}

void usart3_rx_irq(void)
{

}

void uart4_rx_irq(void)
{

}

void uart5_rx_irq(void)
{

}

void uart6_rx_irq(void)
{

}

void uart7_rx_irq(void)
{

}

void uart8_rx_irq(void)
{

}

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/*********************************************************************
 * @fn      NMI_Handler
 *
 * @brief   This function handles NMI exception.
 *
 * @return  none
 */
void NMI_Handler(void)
{
}

/*********************************************************************
 * @fn      HardFault_Handler
 *
 * @brief   This function handles Hard Fault exception.
 *
 * @return  none
 */
void HardFault_Handler(void)
{
  while (1)
  {
  }
}
