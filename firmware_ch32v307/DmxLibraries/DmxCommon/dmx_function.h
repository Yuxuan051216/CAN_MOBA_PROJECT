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
 * @file       dmx_function.h
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

#ifndef __DMX_FUNCTION_H
#define __DMX_FUNCTION_H

/**
*
* @brief    绝对值函数
* @param    int                 整数
* @return   int                 绝对值
* @notes
* Example:  abs_func(-2);
*
**/
int abs_func(int x);

/**
*
* @brief    限幅函数
* @param    x                   需要限幅的数据
* @param    min                 限幅的最小值
* @param    max                 限幅的最大值
* @return   int                 限幅值
* @notes
* Example:  limit_func(110,-100,100);   // 返回值为100
*
**/
int limit_func(int x,int min,int max);

/**
*
* @brief    整数转字符串
* @param    dat                 数据
* @param    str                 字符串指针
* @return   void
* @notes
* Example:  int_to_str_func(-521,str);
*
**/

void int_to_str_func(int dat,char *str);
/**
*
* @brief    浮点数转字符串
* @param    dat                 数据
* @param    pointnum            小数点位数
* @param    str                 字符串指针
* @return   void
* @notes
* Example:  float_to_str_func(-521.22,2,str);
*
**/
void float_to_str_func(double dat,unsigned char pointnum,char *str);

#endif /* __DMX_FUNCTION_H */
