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
 * @file       dmx_gpio.c
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

#include "dmx_gpio.h"

GPIO_TypeDef *GPIOx[5] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOE};

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
void init_gpio(GPIO_pin_enum pin, GPIO_mode_enum pinMode, GPIO_speed_enum pinSpeed, unsigned char level)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    unsigned char port = pin/16;
    switch(port)
    {
        case 0: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);break;
        case 1: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);break;
        case 2: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);break;
        case 3: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);break;
        case 4: RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);break;
    }
    GPIO_InitStructure.GPIO_Pin  = 1 << (pin%16);
    GPIO_InitStructure.GPIO_Mode = pinMode;
    if(pinMode >> 4  == 0x01)
        GPIO_InitStructure.GPIO_Speed = pinSpeed;
    GPIO_Init(GPIOx[port], &GPIO_InitStructure);
    if(pinMode >> 4  == 0x01)
        GPIO_WriteBit(GPIOx[port], GPIO_InitStructure.GPIO_Pin, level);
}

/**
*
* @brief    GPIO模式切换
* @param    pinIndex    该端口的对应引脚号
* @param    pinMode     引脚模式配置(可设置参数,由dmx_gpio.h中GPIO引脚模式枚举值确定)
* @param    pinSpeed    引脚速率配置(可设置参数,由dmx_gpio.h中GPIO引脚速率枚举值确定)
* @return   void
* @notes
* Example:  set_dir_gpio( A0 , GPO_PP , Speed_50MHZ );
*
**/
void set_dir_gpio(GPIO_pin_enum pinIndex, GPIO_mode_enum pinMode, GPIO_speed_enum pinSpeed)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = 1 << (pinIndex%16);
    GPIO_InitStructure.GPIO_Mode = pinMode;
    if(pinMode >> 4  == 0x01)
        GPIO_InitStructure.GPIO_Speed = pinSpeed;
    GPIO_Init(GPIOx[pinIndex/16], &GPIO_InitStructure);
}

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
void set_level_gpio(GPIO_pin_enum pin, unsigned char level)
{
    if(level)
    {
        GPIOx[pin/16]->BSHR = 1 << (pin%16);
    }
    else
    {
        GPIOx[pin/16]->BCR = 1 << (pin%16);
    }
}

/**
*
* @brief    GPIO状态获取
* @param    pin             该端口的对应引脚号
* @notes
* Example:  unsigned char status = get_level_gpio(A0);
*
**/
unsigned char get_level_gpio(GPIO_pin_enum pin)
{
    if((GPIOx[pin/16]->INDR & (1 << (pin%16))) > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
*
* @brief    GPIO状态翻转
* @param    pin         该端口的对应引脚号
* @return   void
* @notes
* Example:  toggle_level_gpio(A0);
*
**/
void toggle_level_gpio(GPIO_pin_enum pin)
{
    GPIOx[pin/16]->OUTDR ^= (1 << (pin%16));
}
