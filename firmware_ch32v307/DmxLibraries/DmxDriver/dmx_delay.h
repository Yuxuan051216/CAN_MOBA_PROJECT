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
 * @file       dmx_delay.h
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

#ifndef __DMX_DELAY_H
#define __DMX_DELAY_H

#include "ch32v30x_rcc.h"

/**
*
* @brief    systick延时函数
* @param    time            需要延时的时间(单位：us)
* @return   void
* @notes    中断里不要调用延时函数
* Example:  delay_systick(1000);
*
**/
void delay_systick(unsigned long time);

/**
*
* @brief    systick定时器启动
* @param    void
* @return   void
* @notes    使用systick计数器计算程序运行时间时,在该程序段中不可有延时函数
* Example:  start_systick();
*
**/
void start_systick(void);

/**
*
* @brief    获取从start_systick();语句后到此处的时间
* @param    void
* @return   unsigned long   返回从start_systick()函数开始执行到现在的时间(单位为1us)
* @notes    使用systick计数器计算程序运行时间时,在该程序段中不可有延时函数
* Example:  unsigned long time = get_systick();     // 此time为start_systick()函数到此函数处的时间间隔
*
**/
unsigned long get_systick(void);

// 以下宏定义用于延时
#define Delay_ms(time)    delay_systick(time*1000) // 设置延时时间(单位ms)
#define Delay_us(time)    delay_systick(time)      // 设置延时时间(单位us)

#endif /* __DMX_DELAY_H */
