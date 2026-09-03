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
 * @file       dmx_pwm.c
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

#include "dmx_pwm.h"

/**
*
* @brief    初始化引脚作为pwm输出引脚，并设定其初始频率及初始占空比
* @param    pwmpin              PWM引脚
* @param    freq                PWM频率
* @param    duty                PWM占空比
* @return   void
* @notes
* Example:  init_pwm( TIM1_RM0_CH1_A8 , 13333 , 1000);
*
**/
void init_pwm(PWM_pin_enum pwmpin, unsigned int freq, unsigned int duty)
{
    TIM_TypeDef *timn = 0;
    TIM_OCInitTypeDef TIM_OCInitStructure={0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
    unsigned short tim_prescaler = (unsigned short)((SystemCoreClock/freq) >> 16);
    unsigned short tim_period = (unsigned short)((SystemCoreClock/freq)/(tim_prescaler + 1));
    unsigned short tim_pulse = (unsigned short)(tim_period*duty/MAX_DUTY);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    switch((pwmpin&0xF0000)>>16)
    {
        case 1:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);timn = TIM1;break;
        case 2:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);timn = TIM2;break;
        case 3:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);timn = TIM3;break;
        case 4:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);timn = TIM4;break;
        case 5:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);timn = TIM5;break;
        case 8:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);timn = TIM8;break;
        case 9:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);timn = TIM9;break;
        case 10:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);timn = TIM10;break;
    }
    switch((pwmpin&0xFF000)>>12)
    {
        case 0x11:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1 , ENABLE);break;
        case 0x13:GPIO_PinRemapConfig(GPIO_FullRemap_TIM1    , ENABLE);break;
        case 0x21:GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);break;
        case 0x22:GPIO_PinRemapConfig(GPIO_PartialRemap2_TIM2, ENABLE);break;
        case 0x23:GPIO_PinRemapConfig(GPIO_FullRemap_TIM2    , ENABLE);break;
        case 0x32:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3 , ENABLE);break;
        case 0x33:GPIO_PinRemapConfig(GPIO_FullRemap_TIM3    , ENABLE);break;
        case 0x41:GPIO_PinRemapConfig(GPIO_Remap_TIM4        , ENABLE);break;
        case 0x81:GPIO_PinRemapConfig(GPIO_Remap_TIM8        , ENABLE);break;
        case 0x91:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM9 , ENABLE);break;
        case 0x93:GPIO_PinRemapConfig(GPIO_FullRemap_TIM9    , ENABLE);break;
        case 0xA1:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM10, ENABLE);break;
        case 0xA3:GPIO_PinRemapConfig(GPIO_FullRemap_TIM10   , ENABLE);break;
    }

    init_gpio( pwmpin&0x000FF , GPO_AF_PP , Speed_50MHZ , 0);

    TIM_TimeBaseInitStructure.TIM_Period = tim_period - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = tim_prescaler;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(timn, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    if(((pwmpin & 0x00F00)>>8) <= 4)
    {
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
        TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    }
    else
    {
        TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
        TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    }

    TIM_OCInitStructure.TIM_Pulse = tim_pulse;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    switch((pwmpin&0x00F00)>>8)
    {
        case 1:TIM_OC1Init(timn, &TIM_OCInitStructure); TIM_OC1PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 2:TIM_OC2Init(timn, &TIM_OCInitStructure); TIM_OC2PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 3:TIM_OC3Init(timn, &TIM_OCInitStructure); TIM_OC3PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 4:TIM_OC4Init(timn, &TIM_OCInitStructure); TIM_OC4PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 5:TIM_OC1Init(timn, &TIM_OCInitStructure); TIM_OC1PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 6:TIM_OC2Init(timn, &TIM_OCInitStructure); TIM_OC2PreloadConfig(timn, TIM_OCPreload_Enable);break;
        case 7:TIM_OC3Init(timn, &TIM_OCInitStructure); TIM_OC3PreloadConfig(timn, TIM_OCPreload_Enable);break;
    }

    TIM_CtrlPWMOutputs(timn, ENABLE );
    TIM_ARRPreloadConfig(timn, ENABLE );
    TIM_Cmd(timn, ENABLE );
}

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
void set_duty_pwm(PWM_pin_enum pwmpin, unsigned int duty)
{
    TIM_TypeDef *timn = 0;
    unsigned short tim_period = 0,tim_pulse = 0;

    switch((pwmpin&0xF0000)>>16)
    {
        case 1:timn = TIM1;break;
        case 2:timn = TIM2;break;
        case 3:timn = TIM3;break;
        case 4:timn = TIM4;break;
        case 5:timn = TIM5;break;
        case 8:timn = TIM8;break;
        case 9:timn = TIM9;break;
        case 10:timn = TIM10;break;
    }
    tim_period = timn->ATRLR;
    tim_pulse = tim_period * duty / MAX_DUTY;

    switch((pwmpin&0x00F00)>>8)
    {
        case 1:case 5:timn->CH1CVR = tim_pulse;break;
        case 2:case 6:timn->CH2CVR = tim_pulse;break;
        case 3:case 7:timn->CH3CVR = tim_pulse;break;
        case 4:timn->CH4CVR = tim_pulse;break;
    }
}
