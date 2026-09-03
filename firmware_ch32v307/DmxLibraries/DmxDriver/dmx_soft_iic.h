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
 * @file       dmx_soft_iic.h
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

#ifndef __DMX_SOFT_IIC_H
#define __DMX_SOFT_IIC_H

#include "dmx_gpio.h"

#define ACK     1   // 主应答
#define NACK    0   // 从应答

typedef struct
{
    GPIO_pin_enum       scl_pin;
    GPIO_pin_enum       sda_pin;
    unsigned int        delay;
}SOFT_IIC_struct;

/**
*
* @brief    软件IIC引脚初始化
* @param    soft_iic_obj        IIC结构体
* @param    iic_delay           IIC延时
* @param    scl_pin             SCL引脚
* @param    sda_pin             SCL引脚
* @return   void
* @notes
* Example:  init_soft_iic(&MPU, 200, MPU_SCL_PIN, MPU_SDA_PIN);
*
**/
void init_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned int iic_delay,GPIO_pin_enum scl_pin, GPIO_pin_enum sda_pin);

/**
*
* @brief    写数据到设备寄存器中
* @param    soft_iic_obj        IIC结构体
* @param    device_addr         设备地址
* @param    reg_addr            设备寄存器地址
* @param    data                数据
* @return   void
* @notes
* Example:  write_data_soft_iic(&MPU,addr,reg,data)
*
**/
void write_data_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned char device_addr, unsigned char reg_addr, unsigned char data);

/**
*
* @brief    从tof设备寄存器写多字节数据
* @param    soft_iic_obj        IIC结构体
* @param    device_addr         设备地址
* @param    reg_addr            设备寄存器地址
* @param    data_addr           数据地址
* @param    num                 数据长度
* @return   void
* @notes
* Example:  write_datas_tof_soft_iic(&TOF400C,addr,reg,data,num)
*
**/
void write_datas_tof_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned int device_addr, unsigned int reg_addr, unsigned char* data_addr, unsigned char num);

/**
*
* @brief    从设备寄存器读数据
* @param    soft_iic_obj        IIC结构体
* @param    device_addr         设备地址
* @param    reg_addr            设备寄存器地址
* @return   unsigned char
* @notes
* Example:  read_data_soft_iic(&MPU,addr,reg,data)
*
**/
unsigned char read_data_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned char device_addr, unsigned char reg_addr);

/**
*
* @brief    从设备寄存器读取多字节数据
* @param    soft_iic_obj        IIC结构体
* @param    device_addr         设备地址
* @param    reg_addr            设备寄存器地址
* @param    data_addr           数据地址
* @param    num                 数据长度
* @return   void
* @notes
* Example:  read_datas_soft_iic(&MPU,addr,reg,data,num)
*
**/
void read_datas_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned char device_addr, unsigned char reg_addr, unsigned char* data_addr, unsigned char num);

/**
*
* @brief    从tof设备寄存器读取多字节数据
* @param    soft_iic_obj        IIC结构体
* @param    device_addr         设备地址
* @param    reg_addr            设备寄存器地址
* @param    data_addr           数据地址
* @param    num                 数据长度
* @return   void
* @notes
* Example:  read_datas_tof_soft_iic(&TOF400C,addr,reg,data,num)
*
**/
void read_datas_tof_soft_iic(SOFT_IIC_struct *soft_iic_obj,unsigned int device_addr, unsigned int reg_addr, unsigned char* data_addr, unsigned char num);

#endif /* __DMX_SOFT_IIC_H */
