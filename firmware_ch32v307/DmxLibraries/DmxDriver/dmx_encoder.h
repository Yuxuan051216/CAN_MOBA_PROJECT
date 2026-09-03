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
 * @file       dmx_encoder.h
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

#ifndef __DMX_ENCODER_H
#define __DMX_ENCODER_H

#include "dmx_gpio.h"

// 编码器引脚枚举
typedef enum
{
    // 定时器1
    TIM1_RM0_DIR_A8_COUNT_A9  = 0x100809,
    TIM1_RM3_DIR_E9_COUNT_E11 = 0x13494B,

    // 定时器2
    TIM2_RM0_DIR_A0_COUNT_A1  = 0x200001,
    TIM2_RM1_DIR_A15_COUNT_B3 = 0x210F13,

    // 定时器3
    TIM3_RM0_DIR_A6_COUNT_A7  = 0x300607,
    TIM3_RM2_DIR_B4_COUNT_B5  = 0x321415,
    TIM3_RM3_DIR_C6_COUNT_C7  = 0x332627,

    // 定时器4
    TIM4_RM0_DIR_B6_COUNT_B7  = 0x401617,
    TIM4_RM1_DIR_D12_COUNT_D13= 0x413C3D,

    // 定时器5
    TIM5_RM0_DIR_A0_COUNT_A1  = 0x500001,

    // 定时器8
    TIM8_RM0_DIR_C6_COUNT_C7  = 0x802627,
    TIM8_RM1_DIR_B6_COUNT_B7  = 0x811617,

    // 定时器9
    TIM9_RM0_DIR_A2_COUNT_A3  = 0x900203,
    TIM9_RM3_DIR_D9_COUNT_D11 = 0x93393B,

    // 定时器10
    TIMA_RM0_DIR_B8_COUNT_B9  = 0xA01819,
    TIMA_RM1_DIR_B3_COUNT_B4  = 0xA11314,
    TIMA_RM3_DIR_D1_COUNT_D3  = 0xA33133,

}ENCODER_pin_enum;

/**
*
* @brief    初始化编码器引脚
* @param    encoderpin      编码器B相引脚作为方向,编码器A相引脚作为计数
* @param    mode            0:AB相编码器;1:带方向编码器
* @return   void
* @notes
* Example:  Init_Encoder( TIM9_RM3_DIR_D9_COUNT_D11,1);
*
**/
void Init_Encoder(ENCODER_pin_enum encoderpin , unsigned char mode);

/**
*
* @brief    获取编码器计数值
* @param    encoderpin   编码器B相引脚作为方向,编码器A相引脚作为计数
* @return   void
* @notes
* Example:  Get_Encoder(TIM9_RM3_DIR_D9_COUNT_D11);
*
**/
short Get_Encoder(ENCODER_pin_enum encoderpin);

/**
*
* @brief    清除编码器计数值
* @param    encoderpin   编码器B相引脚作为方向,编码器A相引脚作为计数
* @return   void
* @notes
* Example:  Clear_Encoder(TIM9_RM3_DIR_D9_COUNT_D11);
*
**/
void Clear_Encoder(ENCODER_pin_enum encoderpin);

#endif /* __DMX_ENCODER_H */
