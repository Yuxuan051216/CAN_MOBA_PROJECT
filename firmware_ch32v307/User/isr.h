/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_it.h
* Author             : WCH
* Version            : V1.0.0
* Date               : 2021/06/06
* Description        : This file contains the headers of the interrupt handlers.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/
#ifndef __ISR_H
#define __ISR_H

#include "../QhLibraries/Debug/debug.h"

// 声明定时器中断服务函数
extern void pit1_irq(void);
extern void pit2_irq(void);
extern void pit3_irq(void);
extern void pit4_irq(void);
extern void pit5_irq(void);
extern void pit6_irq(void);
extern void pit7_irq(void);
extern void pit8_irq(void);
extern void pit9_irq(void);
extern void pit10_irq(void);

// 声明外部中断服务函数
extern void exti0_irq(void);
extern void exti1_irq(void);
extern void exti2_irq(void);
extern void exti3_irq(void);
extern void exti4_irq(void);
extern void exti5_irq(void);
extern void exti6_irq(void);
extern void exti7_irq(void);
extern void exti8_irq(void);
extern void exti9_irq(void);
extern void exti10_irq(void);
extern void exti11_irq(void);
extern void exti12_irq(void);
extern void exti13_irq(void);
extern void exti14_irq(void);
extern void exti15_irq(void);

// 声明串口接收函数
extern void usart1_rx_irq(void);
extern void usart2_rx_irq(void);
extern void usart3_rx_irq(void);
extern void uart4_rx_irq(void);
extern void uart5_rx_irq(void);
extern void uart6_rx_irq(void);
extern void uart7_rx_irq(void);
extern void uart8_rx_irq(void);

#endif /* __ISR_H */


