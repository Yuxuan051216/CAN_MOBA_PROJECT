/****************************************************************************************
 *     COPYRIGHT NOTICE
 *     Copyright (C) 2024,AS DAIMXA
 *     copyright Copyright (C) 呆萌侠DAIMXA,2024
 *     All rights reserved.
 *     技术讨论QQ群：710026750
 *
 *     除注明出处外，以下所有内容版权均属呆萌侠智能科技所有，未经允许，不得用于商业用途，
 *     修改内容时必须保留呆萌智能侠科技的版权声明。
 *      ____    _    ___ __  ____  __    _
 *     |  _ \  / \  |_ _|  \/  \ \/ /   / \
 *     | | | |/ _ \  | || |\/| |\  /   / _ \
 *     | |_| / ___ \ | || |  | |/  \  / ___ \
 *     |____/_/   \_\___|_|  |_/_/\_\/_/   \_\
 *
 * @file       dmx_pit.c
 * @brief      呆萌侠CH32V307VCT6开源库
 * @company    合肥呆萌侠智能科技有限公司
 * @author     呆萌侠科技（QQ：2453520483）
 * @MCUcore    CH32V307VCT6
 * @Software   MounRicer Stdio V191
 * @version    查看说明文档内version版本说明
 * @Taobao     https://daimxa.taobao.com/
 * @Openlib    https://gitee.com/daimxa
 * @date       2024-01-04
****************************************************************************************/

#include "dmx_pit.h"

void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM6_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM7_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM8_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM9_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM10_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/**
*
* @brief    定时器中断初始化
* @param    pitn        选择TIM模块
* @param    time        定时时间(主频144MHZ时1us~29820000us)
* @return   void
* @notes    单位:us
* Example:  init_pit( PIT1 , 1 );  // TIME1定时器定时时间1us
*
**/
void init_pit(TIM_enum pitn, unsigned int time)
{
    unsigned char irqn;
    TIM_TypeDef *timn = 0;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    if(time > ((1.0/(SystemCoreClock/1000000)) * 65535 * 65535))
        time = ((1.0/(SystemCoreClock/1000000)) * 65535 * 65535);
    unsigned int period = (SystemCoreClock/1000000) * time;
    unsigned short tim_prescaler = (unsigned short)(period >> 16);
    unsigned short tim_period = (unsigned short)(period / (tim_prescaler + 1));

    switch(pitn%10)
    {
        case 0:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);timn = TIM10;irqn = 95;break;
        case 1:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);timn = TIM1;irqn = 41;break;
        case 2:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);timn = TIM2;irqn = 44;break;
        case 3:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);timn = TIM3;irqn = 45;break;
        case 4:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);timn = TIM4;irqn = 46;break;
        case 5:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);timn = TIM5;irqn = 66;break;
        case 6:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);timn = TIM6;irqn = 70;break;
        case 7:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);timn = TIM7;irqn = 71;break;
        case 8:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);timn = TIM8;irqn = 60;break;
        case 9:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);timn = TIM9;irqn = 91;break;
    }

    TIM_TimeBaseInitStructure.TIM_Period = tim_period;
    TIM_TimeBaseInitStructure.TIM_Prescaler = tim_prescaler;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(timn, &TIM_TimeBaseInitStructure);
    TIM_ITConfig(timn,TIM_IT_Update,ENABLE );

    TIM_ClearITPendingBit(timn, TIM_IT_Update);

    NVIC_SetPriority(irqn, 3);
    NVIC_EnableIRQ(irqn);
    TIM_Cmd(timn, ENABLE);
}

/**
*
* @brief    定时器中断使能
* @param    pitn        选择TIM模块
* @return   void
* @notes
* Example:  enable_pit(PIT1);
*
**/
void enable_pit(TIM_enum pitn)
{
    const unsigned char irqn[10]={95,41,44,45,46,66,70,71,60,91};
    NVIC_EnableIRQ(irqn[pitn%10]);
}

/**
*
* @brief    定时器中断禁止
* @param    pitn        选择TIM模块
* @return   void
* @notes
* Example:  disable_pit(PIT1);
*
**/
void disable_pit(TIM_enum pitn)
{
    const unsigned char irqn[10]={95,41,44,45,46,66,70,71,60,91};
    NVIC_DisableIRQ(irqn[pitn%10]);
}

/**
*
* @brief    TIM中断服务函数示例
* @param
* @return
* @notes
* Example:
*
**/
void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
        pit1_irq();
    }
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        pit2_irq();
    }
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        pit3_irq();
    }
}

void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        pit4_irq();
    }
}

void TIM5_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
        pit5_irq();
    }
}

void TIM6_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
        pit6_irq();
    }
}

void TIM7_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
        pit7_irq();
    }
}

void TIM8_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM8, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM8, TIM_IT_Update);
        pit8_irq();
    }
}

void TIM9_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
        pit9_irq();
    }
}

void TIM10_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM10, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
        pit10_irq();
    }
}
