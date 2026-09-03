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
 * @file       dmx_adc.c
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

#include "dmx_adc.h"

/**
*
* @brief    adc初始化
* @param    adc_pin     选择需要初始化adc引脚(dmx_adc.h文件里已枚举定义)
* @return   void
* @notes    调用此函数前可查看dmx_adc.h文件里枚举的可用引脚
* Example:  init_adc(ADC1_CH0_A0);
*
**/
void init_adc(ADC_pin_enum adc_pin)
{
    ADC_TypeDef *adcx = 0;
    ADC_InitTypeDef ADC_InitStructure = {0};
    if((adc_pin&0xF000)>>12 == 1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
        adcx = ADC1;
    }
    else
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
        adcx = ADC2;
    }

    init_gpio( adc_pin , GPI_AIN , Speed_50MHZ , 0);

    RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_InitStructure.ADC_OutputBuffer = ADC_OutputBuffer_Disable;
    ADC_InitStructure.ADC_Pga = ADC_Pga_1;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(adcx, &ADC_InitStructure);

    ADC_Cmd(adcx, ENABLE);
    ADC_BufferCmd(adcx, DISABLE);
    ADC_ResetCalibration(adcx);
    while(ADC_GetResetCalibrationStatus(adcx));
    ADC_StartCalibration(adcx);
    while(ADC_GetCalibrationStatus(adcx));
    ADC_BufferCmd(adcx, ENABLE);
}

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
unsigned short get_adc(ADC_pin_enum adc_pin, ADC_res_enum adc_res)
{
    ADC_TypeDef *adcx = 0;
    if((adc_pin&0xF000)>>12 == 1)
    {
        adcx = ADC1;
    }
    else
    {
        adcx = ADC2;
    }
    ADC_RegularChannelConfig(adcx, (adc_pin&0x0F00)>>8, 1, ADC_SampleTime_41Cycles5);
    ADC_SoftwareStartConvCmd(adcx, ENABLE);
    while(!ADC_GetFlagStatus(adcx, ADC_FLAG_EOC ));
    return ((adcx->RDATAR) >> adc_res );
}
