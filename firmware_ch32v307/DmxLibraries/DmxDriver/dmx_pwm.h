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
 * @file       dmx_pwm.h
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

#ifndef __DMX_PWM_H
#define __DMX_PWM_H

#include "dmx_gpio.h"

#define MAX_DUTY    10000

typedef enum
{
    // TIM1_RM0_CH1_A8:定时器1,默认映射,通道1,A8引脚
    // TIM_n:定时器号
    // RM_n:默认映射:0,重映射:1,部分映射:2,完全映射:3
    // CH_n:通道号
    // A8:引脚

    // 注意:同一个定时器模块只能用同一映射方式的一组引脚且频率需要一致
    // 注意:同一映射方式中的N通道和正常通道不能同时使用
    // 如:定时器1默认映射(TIM1_RM0)中A8,A9,A10,A11可以同时使用,B13,B14,B15可以同时使用
    // 但这七个不能同时使用,可以同时使用的引脚使用时要注意频率需要一致否则就以最后设置的频率为准

    // TIM1
    TIM1_RM0_CH1_A8   = 0x10108 ,   TIM1_RM1_CH1_A8  = 0x11108 ,    TIM1_RM3_CH1_E9   = 0x13149 ,
    TIM1_RM0_CH2_A9   = 0x10209 ,   TIM1_RM1_CH2_A9  = 0x11209 ,    TIM1_RM3_CH2_E11  = 0x1324B ,
    TIM1_RM0_CH3_A10  = 0x1030A ,   TIM1_RM1_CH3_A10 = 0x1130A ,    TIM1_RM3_CH3_E13  = 0x1334D ,
    TIM1_RM0_CH4_A11  = 0x1040B ,   TIM1_RM1_CH4_A11 = 0x1140B ,    TIM1_RM3_CH4_E14  = 0x1344E ,
    TIM1_RM0_CH1N_B13 = 0x1051D ,   TIM1_RM1_CH1N_A7 = 0x11507 ,    TIM1_RM3_CH1N_E8  = 0x13548 ,
    TIM1_RM0_CH2N_B14 = 0x1061E ,   TIM1_RM1_CH2N_B0 = 0x11610 ,    TIM1_RM3_CH2N_E10 = 0x1364A ,
    TIM1_RM0_CH3N_B15 = 0x1071F ,   TIM1_RM1_CH3N_B1 = 0x11711 ,    TIM1_RM3_CH3N_E12 = 0x1374C ,

    // TIM2
    TIM2_RM0_CH1_A0   = 0x20100 ,   TIM2_RM1_CH1_A15 = 0x2110F ,    TIM2_RM2_CH1_A0   = 0x22100 ,   TIM2_RM3_CH1_A15  = 0x2310F ,
    TIM2_RM0_CH2_A1   = 0x20201 ,   TIM2_RM1_CH2_B3  = 0x21213 ,    TIM2_RM2_CH2_A1   = 0x22201 ,   TIM2_RM3_CH2_B3   = 0x23213 ,
    TIM2_RM0_CH3_A2   = 0x20302 ,   TIM2_RM1_CH3_A2  = 0x21302 ,    TIM2_RM2_CH3_B10  = 0x2231A ,   TIM2_RM3_CH3_B10  = 0x2331A ,
    TIM2_RM0_CH4_A3   = 0x20403 ,   TIM2_RM1_CH4_A3  = 0x21403 ,    TIM2_RM2_CH4_B11  = 0x2241B ,   TIM2_RM3_CH4_B11  = 0x2341B ,

    // TIM3
    TIM3_RM0_CH1_A6   = 0x30106 ,   TIM3_RM2_CH1_B4  = 0x32114 ,    TIM3_RM3_CH1_C6   = 0x33126 ,
    TIM3_RM0_CH2_A7   = 0x30207 ,   TIM3_RM2_CH2_B5  = 0x32215 ,    TIM3_RM3_CH2_C7   = 0x33227 ,
    TIM3_RM0_CH3_B0   = 0x30310 ,   TIM3_RM2_CH3_B0  = 0x32310 ,    TIM3_RM3_CH3_C8   = 0x33328 ,
    TIM3_RM0_CH4_B1   = 0x30411 ,   TIM3_RM2_CH4_B1  = 0x32411 ,    TIM3_RM3_CH4_C9   = 0x33429 ,

    // TIM4
    TIM4_RM0_CH1_B6   = 0x40116 ,   TIM4_RM1_CH1_D12 = 0x4113C ,
    TIM4_RM0_CH2_B7   = 0x40217 ,   TIM4_RM1_CH2_D13 = 0x4123D ,
    TIM4_RM0_CH3_B8   = 0x40318 ,   TIM4_RM1_CH3_D14 = 0x4133E ,
    TIM4_RM0_CH4_B9   = 0x40419 ,   TIM4_RM1_CH4_D15 = 0x4143F ,

    // TIM5
    TIM5_RM0_CH1_A0   = 0x50100 ,
    TIM5_RM0_CH2_A1   = 0x50201 ,
    TIM5_RM0_CH3_A2   = 0x50302 ,
    TIM5_RM0_CH4_A3   = 0x50403 ,

    // TIM8
    TIM8_RM0_CH1_C6   = 0x80126 ,   TIM8_RM1_CH1_B6  = 0x81116 ,
    TIM8_RM0_CH2_C7   = 0x80227 ,   TIM8_RM1_CH2_B7  = 0x81217 ,
    TIM8_RM0_CH3_C8   = 0x80328 ,   TIM8_RM1_CH3_B8  = 0x81318 ,
    TIM8_RM0_CH4_C9   = 0x80429 ,   TIM8_RM1_CH4_C13 = 0x8142D ,
    TIM8_RM0_CH1N_A7  = 0x80507 ,   TIM8_RM1_CH1N_A13= 0x8150D ,
    TIM8_RM0_CH2N_B0  = 0x80610 ,   TIM8_RM1_CH2N_A14= 0x8160E ,
    TIM8_RM0_CH3N_B1  = 0x80711 ,   TIM8_RM1_CH3N_A15= 0x8170F ,

    // TIM9
    TIM9_RM0_CH1_A2   = 0x90102 ,   TIM9_RM1_CH1_A2  = 0x91102 ,    TIM9_RM3_CH1_D9   = 0x93139 ,
    TIM9_RM0_CH2_A3   = 0x90203 ,   TIM9_RM1_CH2_A3  = 0x91203 ,    TIM9_RM3_CH2_D11  = 0x9323B ,
    TIM9_RM0_CH3_A4   = 0x90304 ,   TIM9_RM1_CH3_A4  = 0x91304 ,    TIM9_RM3_CH3_D13  = 0x9333D ,
    TIM9_RM0_CH4_C4   = 0x90424 ,   TIM9_RM1_CH4_C14 = 0x9142E ,    TIM9_RM3_CH4_D15  = 0x9343F ,
    TIM9_RM0_CH1N_C0  = 0x90520 ,   TIM9_RM1_CH1N_B0 = 0x91510 ,    TIM9_RM3_CH1N_D8  = 0x93538 ,
    TIM9_RM0_CH2N_C1  = 0x90621 ,   TIM9_RM1_CH2N_B1 = 0x91611 ,    TIM9_RM3_CH2N_D10 = 0x9363A ,
    TIM9_RM0_CH3N_C2  = 0x90722 ,   TIM9_RM1_CH3N_B2 = 0x91712 ,    TIM9_RM3_CH3N_D12 = 0x9373C ,

    // TIM10
    TIMA_RM0_CH1_B8   = 0xA0118 ,   TIMA_RM1_CH1_B3  = 0xA1113 ,    TIMA_RM3_CH1_D1   = 0xA3131 ,
    TIMA_RM0_CH2_B9   = 0xA0219 ,   TIMA_RM1_CH2_B4  = 0xA1214 ,    TIMA_RM3_CH2_D3   = 0xA3233 ,
    TIMA_RM0_CH3_C3   = 0xA0323 ,   TIMA_RM1_CH3_B5  = 0xA1315 ,    TIMA_RM3_CH3_D5   = 0xA3335 ,
    TIMA_RM0_CH4_C11  = 0xA042B ,   TIMA_RM1_CH4_C15 = 0xA142F ,    TIMA_RM3_CH4_D7   = 0xA3437 ,
    TIMA_RM0_CH1N_A12 = 0xA050C ,   TIMA_RM1_CH1N_A5 = 0xA1505 ,    TIMA_RM3_CH1N_E3  = 0xA3543 ,
    TIMA_RM0_CH2N_A13 = 0xA060D ,   TIMA_RM1_CH2N_A6 = 0xA1606 ,    TIMA_RM3_CH2N_E4  = 0xA3644 ,
    TIMA_RM0_CH3N_A14 = 0xA070E ,   TIMA_RM1_CH3N_A7 = 0xA1707 ,    TIMA_RM3_CH3N_E5  = 0xA3745 ,

}PWM_pin_enum;

/**
*
* @brief    初始化引脚作为pwm输出引脚，并设定其初始频率及初始占空比
* @param    pwmpin              PWM引脚
* @param    freq                PWM频率
* @param    duty                PWM占空比
* @return   void
* @notes
* Example:  init_pwm( TIM1_RM0_CH1_A8 , 12222 , 1000);
*
**/
void init_pwm(PWM_pin_enum pwmpin, unsigned int freq, unsigned int duty);

/**
*
* @brief    设置输出占空比
* @param    pwmpin              PWM引脚
* @param    duty                PWM占空比
* @return   void
* @notes    duty最大值为10000即占空比100%
* Example:  set_duty_pwm( TIM1_RM0_CH1_A8 , 2000);
*
**/
void set_duty_pwm(PWM_pin_enum pwmpin, unsigned int duty);

#endif /* __DMX_PWM_H */
