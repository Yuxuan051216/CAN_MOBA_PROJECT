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
 * @file       dmx_adc.h
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

#ifndef __DMX_ADC_H
#define __DMX_ADC_H

#include "dmx_gpio.h"
#include "ch32v30x_adc.h"

// ADC引脚枚举
typedef enum
{
    // ADC采样源1,16通道
    ADC1_CH0_A0 = 0x1000,
    ADC1_CH1_A1 = 0x1101,
    ADC1_CH2_A2 = 0x1202,
    ADC1_CH3_A3 = 0x1303,
    ADC1_CH4_A4 = 0x1404,
    ADC1_CH5_A5 = 0x1505,
    ADC1_CH6_A6 = 0x1606,
    ADC1_CH7_A7 = 0x1707,
    ADC1_CH8_B0 = 0x1810,
    ADC1_CH9_B1 = 0x1911,
    ADC1_CHA_C0 = 0x1A20,
    ADC1_CHB_C1 = 0x1B21,
    ADC1_CHC_C2 = 0x1C22,
    ADC1_CHD_C3 = 0x1D23,
    ADC1_CHE_C4 = 0x1E24,
    ADC1_CHF_C5 = 0x1F25,

    // ADC采样源2,16通道
    ADC2_CH0_A0 = 0x2000,
    ADC2_CH1_A1 = 0x2101,
    ADC2_CH2_A2 = 0x2202,
    ADC2_CH3_A3 = 0x2303,
    ADC2_CH4_A4 = 0x2404,
    ADC2_CH5_A5 = 0x2505,
    ADC2_CH6_A6 = 0x2606,
    ADC2_CH7_A7 = 0x2707,
    ADC2_CH8_B0 = 0x2810,
    ADC2_CH9_B1 = 0x2911,
    ADC2_CHA_C0 = 0x2A20,
    ADC2_CHB_C1 = 0x2B21,
    ADC2_CHC_C2 = 0x2C22,
    ADC2_CHD_C3 = 0x2D23,
    ADC2_CHE_C4 = 0x2E24,
    ADC2_CHF_C5 = 0x2F25,

}ADC_pin_enum;

// ADC分辨率枚举
typedef enum
{
    ADC8BIT  = 4,    // 8位分辨率     获取ADC范围为0~255
    ADC10BIT = 2,    // 10位分辨率  获取ADC范围为0~1023
    ADC12BIT = 0,    // 12位分辨率  获取ADC范围为0~4095
}ADC_res_enum;

/**
*
* @brief    adc初始化
* @param    adc_pin     选择需要初始化adc引脚(dmx_adc.h文件里已枚举定义)
* @return   void
* @notes    调用此函数前可查看dmx_adc.h文件里枚举的可用引脚
* Example:  init_adc(ADC1_CH0_A0);
*
**/
void init_adc(ADC_pin_enum adc_pin);

/**
*
* @brief    adc转换一次即获取一次ad值
* @param    adc_pin     选择需要读取的adc引脚(dmx_adc.h文件里已枚举定义)
* @param    adc_res     选择adc精度(8bit,10bit,12bit)
* @return   void
* @notes
* Example:  get_adc( ADC1_CH0_A0 , ADC12BIT );
*
**/
unsigned short get_adc(ADC_pin_enum adc_pin, ADC_res_enum adc_res);

#endif /* __DMX_ADC_H */
