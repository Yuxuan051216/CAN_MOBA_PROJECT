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
 * @file       dmx_gpio.h
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

#ifndef __DMX_GPIO_H
#define __DMX_GPIO_H

#include "ch32v30x_rcc.h"
#include "ch32v30x_gpio.h"
#include "ch32v30x_exti.h"

// GPIO引脚枚举
typedef enum
{
    A0 , A1 , A2 , A3 , A4 , A5 , A6 , A7 , A8 , A9 , A10 , A11 , A12 , A13 , A14 , A15,
    B0 , B1 , B2 , B3 , B4 , B5 , B6 , B7 , B8 , B9 , B10 , B11 , B12 , B13 , B14 , B15,
    C0 , C1 , C2 , C3 , C4 , C5 , C6 , C7 , C8 , C9 , C10 , C11 , C12 , C13 , C14 , C15,
    D0 , D1 , D2 , D3 , D4 , D5 , D6 , D7 , D8 , D9 , D10 , D11 , D12 , D13 , D14 , D15,
    E0 , E1 , E2 , E3 , E4 , E5 , E6 , E7 , E8 , E9 , E10 , E11 , E12 , E13 , E14 , E15,
}GPIO_pin_enum;

// GPIO引脚模式枚举
typedef enum
{
    GPI_AIN         = 0x00, // 模拟输入模式
    GPI_FLOATING    = 0x04, // 浮空输入模式
    GPI_PD          = 0x28, // 输入下拉模式
    GPI_PU          = 0x48, // 输入上拉模式

    GPO_OD          = 0x14, // 通用开漏输出模式
    GPO_PP          = 0x10, // 通用推挽输出模式
    GPO_AF_OD       = 0x1C, // 复用功能开漏输出模式
    GPO_AF_PP       = 0x18, // 复用功能推挽输出模式
}GPIO_mode_enum;

// GPIO引脚速率枚举
typedef enum
{
    Speed_10MHZ = 1,
    Speed_2MHZ,
    Speed_50MHZ,
}GPIO_speed_enum;

/**
*
* @brief    GPIO初始化
* @param    pin         该端口的对应引脚号
* @param    pinMode     引脚模式配置(可设置参数,由dmx_gpio.h中GPIO引脚模式枚举值确定)
* @param    pinSpeed    引脚速率配置(可设置参数,由dmx_gpio.h中GPIO引脚速率枚举值确定)
* @param    level       引脚初始化时设置的电平状态(输出模式时有效)低电平:0 高电平:1
* @return   void
* @notes
* Example:  init_gpio( A0 , GPO_PP , Speed_50MHZ , 0);
*
**/
void init_gpio(GPIO_pin_enum pin, GPIO_mode_enum pinMode, GPIO_speed_enum pinSpeed, unsigned char level);

/**
*
* @brief    GPIO模式切换
* @param    pin         该端口的对应引脚号
* @param    pinMode     引脚模式配置(可设置参数,由dmx_gpio.h中GPIO引脚模式枚举值确定)
* @param    pinSpeed    引脚速率配置(可设置参数,由dmx_gpio.h中GPIO引脚速率枚举值确定)
* @return   void
* @notes
* Example:  set_dir_gpio( A0 , GPO_PP , Speed_50MHZ );
*
**/
void set_dir_gpio(GPIO_pin_enum pin, GPIO_mode_enum pinMode, GPIO_speed_enum pinSpeed);

/**
*
* @brief    GPIO输出设置
* @param    pin         该端口的对应引脚号
* @param    level       引脚初始化时设置的电平状态(输出模式时有效)低电平:0 高电平:1
* @return   void
* @notes
* Example:  set_level_gpio( A0 , 1);
*
**/
void set_level_gpio(GPIO_pin_enum pin, unsigned char level);

/**
*
* @brief    GPIO状态获取
* @param    pin         该端口的对应引脚号
* @notes
* Example:  unsigned char status = get_level_gpio(A0);
*
**/
unsigned char get_level_gpio(GPIO_pin_enum pin);

/**
*
* @brief    GPIO状态翻转
* @param    pin         该端口的对应引脚号
* @return   void
* @notes
* Example:  toggle_level_gpio(A0);
*
**/
void toggle_level_gpio(GPIO_pin_enum pin);

#endif /* __DMX_GPIO_H */
