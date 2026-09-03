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
 * @file       dmx_exti.h
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

#ifndef __DMX_EXTI_H
#define __DMX_EXTI_H

#include "dmx_gpio.h"

// 触发方式枚举
typedef enum
{
    RISING  = 0x08,     // 上升沿触发
    FALLING = 0x0C,     // 下降沿触发
    BOTH    = 0x10,     // 双边沿触发
}EXTI_mode_enum;

/**
*
* @brief    gpio外部中断初始化
* @param    extiPin         设置gpio外部中断引脚
* @param    mode            设置触发方式
* @return   void
* @notes
* Example:  init_exti(A0, RISING);
*
**/
void init_exti(GPIO_pin_enum extiPin, EXTI_mode_enum mode);

/**
*
* @brief    开启gpio外部中断
* @param    extiPin     设置gpio外部中断引脚
* @return   void
* @notes
* Example:  enable_exti(A2);
*
**/
void enable_exti(GPIO_pin_enum extiPin);

/**
*
* @brief    关闭gpio外部中断
* @param    extiPin     设置gpio外部中断引脚
* @return   void
* @notes
* Example:  disable_exti(A2);
*
**/
void disable_exti(GPIO_pin_enum extiPin);

#endif /* __DMX_EXTI_H */
