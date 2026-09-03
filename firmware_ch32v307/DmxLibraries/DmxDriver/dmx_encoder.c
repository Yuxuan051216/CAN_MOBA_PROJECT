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
 * @file       dmx_encoder.c
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

#include "dmx_encoder.h"

static unsigned char encoder_mode[8] = {0,0,0,0,0,0,0,0};

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
void Init_Encoder(ENCODER_pin_enum encoderpin , unsigned char mode)
{
    TIM_TypeDef *timn = 0;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure={0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    switch((encoderpin&0xF00000)>>20)
    {
        case 1:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);timn = TIM1;encoder_mode[0] = mode;break;
        case 2:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);timn = TIM2;encoder_mode[1] = mode;break;
        case 3:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);timn = TIM3;encoder_mode[2] = mode;break;
        case 4:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);timn = TIM4;encoder_mode[3] = mode;break;
        case 5:RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);timn = TIM5;encoder_mode[4] = mode;break;
        case 8:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8, ENABLE);timn = TIM8;encoder_mode[5] = mode;break;
        case 9:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);timn = TIM9;encoder_mode[6] = mode;break;
        case 10:RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);timn= TIM10;encoder_mode[7] = mode;break;
    }
    switch((encoderpin&0xFF0000)>>16)
    {
        case 0x13:GPIO_PinRemapConfig(GPIO_FullRemap_TIM1    , ENABLE);break;
        case 0x21:GPIO_PinRemapConfig(GPIO_PartialRemap1_TIM2, ENABLE);break;
        case 0x32:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3 , ENABLE);break;
        case 0x33:GPIO_PinRemapConfig(GPIO_FullRemap_TIM3    , ENABLE);break;
        case 0x41:GPIO_PinRemapConfig(GPIO_Remap_TIM4        , ENABLE);break;
        case 0x81:GPIO_PinRemapConfig(GPIO_Remap_TIM8        , ENABLE);break;
        case 0x93:GPIO_PinRemapConfig(GPIO_FullRemap_TIM9    , ENABLE);break;
        case 0xA1:GPIO_PinRemapConfig(GPIO_PartialRemap_TIM10, ENABLE);break;
        case 0xA3:GPIO_PinRemapConfig(GPIO_FullRemap_TIM10   , ENABLE);break;
    }

    init_gpio( (encoderpin&0x0000FF)    , GPI_PU , Speed_50MHZ , 0);
    init_gpio( (encoderpin&0x00FF00)>>8 , GPI_PU , Speed_50MHZ , 0);

    TIM_TimeBaseStructInit(&TIM_TimeBaseInitStructure);
    TIM_TimeBaseInit(timn, &TIM_TimeBaseInitStructure);
    TIM_ITRxExternalClockConfig(timn, TIM_TS_TI2FP2);
    if(mode)
    {
        TIM_ETRConfig(timn, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 15);
    }
    else
    {
        TIM_EncoderInterfaceConfig(timn,TIM_EncoderMode_TI2 ,TIM_ICPolarity_Rising,TIM_ICPolarity_Rising);
    }
    TIM_Cmd(timn, ENABLE);
}

/**
*
* @brief    获取编码器计数值
* @param    encoderpin   编码器B相引脚作为方向,编码器A相引脚作为计数
* @return   void
* @notes
* Example:  Get_Encoder(TIM9_RM3_DIR_D9_COUNT_D11);
*
**/
short Get_Encoder(ENCODER_pin_enum encoderpin)
{
    short result = 0;
    unsigned char mode = 0;
    unsigned char dir_pin = (encoderpin&0x00FF00)>>8;
    switch((encoderpin&0xF00000)>>20)
    {
        case 1:mode = encoder_mode[0];result = TIM1->CNT;break;
        case 2:mode = encoder_mode[1];result = TIM2->CNT;break;
        case 3:mode = encoder_mode[2];result = TIM3->CNT;break;
        case 4:mode = encoder_mode[3];result = TIM4->CNT;break;
        case 5:mode = encoder_mode[4];result = TIM5->CNT;break;
        case 8:mode = encoder_mode[5];result = TIM8->CNT;break;
        case 9:mode = encoder_mode[6];result = TIM9->CNT;break;
        case 10:mode = encoder_mode[7];result= TIM10->CNT;break;
    }
    if(mode)
    {
        if(!get_level_gpio(dir_pin))
        {
            result =- result;
        }
    }
    return result;
}

/**
*
* @brief    清除编码器计数值
* @param    encoderpin   编码器B相引脚作为方向,编码器A相引脚作为计数
* @return   void
* @notes
* Example:  Clear_Encoder(TIM9_RM3_DIR_D9_COUNT_D11);
*
**/
void Clear_Encoder(ENCODER_pin_enum encoderpin)
{
    switch((encoderpin&0xF00000)>>20)
    {
        case 1:TIM1->CNT = 0;break;
        case 2:TIM2->CNT = 0;break;
        case 3:TIM3->CNT = 0;break;
        case 4:TIM4->CNT = 0;break;
        case 5:TIM5->CNT = 0;break;
        case 8:TIM8->CNT = 0;break;
        case 9:TIM9->CNT = 0;break;
        case 10:TIM10->CNT = 0;break;
    }
}
