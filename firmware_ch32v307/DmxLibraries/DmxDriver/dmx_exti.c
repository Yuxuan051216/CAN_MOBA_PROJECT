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
 * @file       dmx_exti.c
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

#include "dmx_exti.h"

void EXTI0_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI1_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI3_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI4_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI9_5_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void EXTI15_10_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

/**
*
* @brief    gpio外部中断初始化
* @param    extiPin         设置gpio外部中断引脚
* @param    mode            设置触发方式
* @return   void
* @notes
* Example:  init_exti(A2, RISING);
*
**/
void init_exti(GPIO_pin_enum extiPin, EXTI_mode_enum mode)
{
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    init_gpio( extiPin , GPIO_Mode_IPU , Speed_50MHZ , 1);
    GPIO_EXTILineConfig(extiPin/16, extiPin%16);

    EXTI_InitStructure.EXTI_Line = 1<<(extiPin%16);
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = mode;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    switch(extiPin%16)
    {
        case 0:NVIC_EnableIRQ(EXTI0_IRQn);break;
        case 1:NVIC_EnableIRQ(EXTI1_IRQn);break;
        case 2:NVIC_EnableIRQ(EXTI2_IRQn);break;
        case 3:NVIC_EnableIRQ(EXTI3_IRQn);break;
        case 4:NVIC_EnableIRQ(EXTI4_IRQn);break;
        case 5:case 6:case 7:case 8:
        case 9:NVIC_EnableIRQ(EXTI9_5_IRQn);break;
        case 10:case 11:case 12:case 13:case 14:
        case 15:NVIC_EnableIRQ(EXTI15_10_IRQn);break;
    }
}

/**
*
* @brief    开启gpio外部中断
* @param    extiPin     设置gpio外部中断引脚
* @return   void
* @notes
* Example:  enable_exti(A2);
*
**/
void enable_exti(GPIO_pin_enum extiPin)
{
    EXTI->INTENR |= 1<<(extiPin%16);
}

/**
*
* @brief    关闭gpio外部中断
* @param    extiPin     设置gpio外部中断引脚
* @return   void
* @notes
* Example:  disable_exti(A2);
*
**/
void disable_exti(GPIO_pin_enum extiPin)
{
    EXTI->INTENR &= ~(1<<(extiPin%16));
}

/**
*
* @brief    GPIO外部中断服务函数示例
* @param
* @return
* @notes
* Example:  直接使用以下函数即可
*
**/
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) == SET)
    {
        exti0_irq();
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) == SET)
    {
        exti1_irq();
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) == SET)
    {
        exti2_irq();
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) == SET)
    {
        exti3_irq();
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) == SET)
    {
        exti4_irq();
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) == SET)
    {
        exti5_irq();
        EXTI_ClearITPendingBit(EXTI_Line5);
    }

    if(EXTI_GetITStatus(EXTI_Line6) == SET)
    {
        exti6_irq();
        EXTI_ClearITPendingBit(EXTI_Line6);
    }

    if(EXTI_GetITStatus(EXTI_Line7) == SET)
    {
        exti7_irq();
        EXTI_ClearITPendingBit(EXTI_Line7);
    }

    if(EXTI_GetITStatus(EXTI_Line8) == SET)
    {
        exti8_irq();
        EXTI_ClearITPendingBit(EXTI_Line8);
    }

    if(EXTI_GetITStatus(EXTI_Line9) == SET)
    {
        exti9_irq();
        EXTI_ClearITPendingBit(EXTI_Line9);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line10) == SET)
    {
        exti10_irq();
        EXTI_ClearITPendingBit(EXTI_Line10);
    }

    if(EXTI_GetITStatus(EXTI_Line11) == SET)
    {
        exti11_irq();
        EXTI_ClearITPendingBit(EXTI_Line11);
    }

    if(EXTI_GetITStatus(EXTI_Line12) == SET)
    {
        exti12_irq();
        EXTI_ClearITPendingBit(EXTI_Line12);
    }

    if(EXTI_GetITStatus(EXTI_Line13) == SET)
    {
        exti13_irq();
        EXTI_ClearITPendingBit(EXTI_Line13);
    }

    if(EXTI_GetITStatus(EXTI_Line14) == SET)
    {
        exti14_irq();
        EXTI_ClearITPendingBit(EXTI_Line14);
    }

    if(EXTI_GetITStatus(EXTI_Line15) == SET)
    {
        exti15_irq();
        EXTI_ClearITPendingBit(EXTI_Line15);
    }
}
