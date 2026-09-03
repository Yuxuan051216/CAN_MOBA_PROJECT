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
 * @file       dmx_pit.h
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

#ifndef __DMX_PIT_H
#define __DMX_PIT_H

#include "ch32v30x_rcc.h"
#include "core_riscv.h"

// TIM模块枚举
typedef enum
{
    PIT10,
    PIT1,
    PIT2,
    PIT3,
    PIT4,
    PIT5,
    PIT6,
    PIT7,
    PIT8,
    PIT9,
}TIM_enum;

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
void init_pit(TIM_enum pitn, unsigned int time);

/**
*
* @brief    定时器中断使能
* @param    pitn        选择TIM模块
* @return   void
* @notes
* Example:  enable_pit(PIT1);
*
**/
void enable_pit(TIM_enum pitn);

/**
*
* @brief    定时器中断禁止
* @param    pitn        选择TIM模块
* @return   void
* @notes
* Example:  disable_pit(PIT1);
*
**/
void disable_pit(TIM_enum pitn);

#endif /* __DMX_PIT_H */
