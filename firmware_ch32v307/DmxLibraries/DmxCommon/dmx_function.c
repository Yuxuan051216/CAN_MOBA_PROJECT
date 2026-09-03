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
 * @file       dmx_function.c
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

#include "dmx_function.h"
#include "string.h"

/**
*
* @brief    绝对值函数
* @param    int                 整数
* @return   int                 绝对值
* @notes
* Example:  abs_func(-2);
*
**/
int abs_func(int x)
{
    return (x >= 0) ? x : -x;
}

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
int limit_func(int x,int min,int max)
{
    return ((x) > (max) ? (max) : ((x) < -(min) ? -(min) : (x)));
}

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
void int_to_str_func(int dat,char *str)
{
    unsigned char dat_buff[12];
    unsigned char num = 0;
    if (str == NULL)
    {
        return;
    }

    if (dat < 0)
    {
        *str++ = '-';
        dat = -dat;
    }
    else if (dat == 0)
    {
        *str++ = '0';
        return;
    }

    while (dat != 0)
    {
        dat_buff[num++] = (dat % 10);
        dat /= 10;
    }

    while (num != 0)
    {
        *str++ = (dat_buff[--num] + '0');
    }
}

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
void float_to_str_func(double dat,unsigned char pointnum,char *str)
{
    int dat_temp = (int)dat;
    int point_dat_temp = 0;
    unsigned char dat_buff[12];
    unsigned char point_dat_buff[9];
    unsigned char num = 0;
    unsigned char point_num = pointnum;

    if (str == NULL)
    {
        return;
    }

    // 数据预处理
    dat = dat - dat_temp;
    if (dat_temp < 0 || dat < 0)
    {
        *str++ = '-';
        dat_temp = -dat_temp;
        dat = -dat;
    }
    else if (dat == 0.0 && dat_temp == 0)
    {
        *str++ = '0';
        *str++ = '.';
        return;
    }

    // 转换浮点数的整数部分
    do
    {
        dat_buff[num++] = (dat_temp % 10);
        dat_temp /= 10;
    }while (dat_temp != 0);
    while (num != 0)
    {
        *str++ = (dat_buff[--num] + '0');
    }

    // 转换浮点数的小数部分
    while(pointnum--)
    {
        dat *= 10;
    }
    point_dat_temp = (int)dat;
    num = 0;
    *str++ = '.';
    if(point_dat_temp == 0)
    {
        *str = '0';
    }
    else
    {
        while (point_num-- != 0)
        {
            point_dat_buff[num++] = (point_dat_temp % 10);
            point_dat_temp /= 10;
        }
        while (num != 0)
        {
            *str++ = (point_dat_buff[--num] + '0');
        }
    }
}
