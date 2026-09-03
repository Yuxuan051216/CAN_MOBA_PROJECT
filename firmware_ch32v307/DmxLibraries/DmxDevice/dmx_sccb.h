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
 * @file       dmx_sccb.h
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

#ifndef __DMX_SCCB_H
#define __DMX_SCCB_H

#include "dmx_gpio.h"

// 摄像头使用SCCB通信,SCCB和IIC基本类似
#define MT9V034_SCL_PIN             D2
#define MT9V034_SDA_PIN             C12

// SCCB接口管脚控制
#define MT9V034_SCL_MODE(dir)       {int count=500;while(--count){__asm("NOP");}}
#define MT9V034_SDA_MODE(dir)       {int count=500;while(--count){__asm("NOP");}}
#define MT9V034_SCL_LEVEL(level)    set_level_gpio(MT9V034_SCL_PIN , level)
#define MT9V034_SDA_LEVEL(level)    set_level_gpio(MT9V034_SDA_PIN , level)
#define MT9V034_SDA_GET             get_level_gpio(MT9V034_SDA_PIN)

// 用户无需调用和更改
#define SCCB_Delay()                {int count=500;while(--count){__asm("NOP");}}

void Set_Exposure_Time(unsigned int exposure,unsigned int analog_gain);
unsigned short Set_Config_MT9V034(int config[], unsigned char mode);

#endif /* __DMX_SCCB_H */
