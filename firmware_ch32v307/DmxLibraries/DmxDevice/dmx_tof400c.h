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
 * @file       dmx_tof400c.h
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

#ifndef __DMX_TOF400C_H
#define __DMX_TOF400C_H

// 测距距离为(0~1.4m)
// 测距频率,可选填100,66,50,30,20,10,5,2(单位Hz)
#define TOF400C_FREQ                100

// TOF400C激光测距模块管脚宏定义
// TOF400C,SCL和SDA管脚
#define TOF400C_SCL_PIN             A7
#define TOF400C_SDA_PIN             B1
// TOF400C引脚电平操作
#define TOF400C_SCL_LEVEL(level)    set_level_gpio(TOF400C_SCL_PIN , level)
#define TOF400C_SDA_LEVEL(level)    set_level_gpio(TOF400C_SDA_PIN , level)
// TOF400C延时
#define TOF400C_DELAY               0
#define TOF400C_DELAY_MS(time)      (Delay_ms(time))
// TOF400C初始化超时次数限制
#define TOF400C_TIMEOUT_COUNT       10


/**
*
* @brief    TOF400C模块初始化
* @param    void
* @return   void
* @notes
* Example:  Init_TOF400C();
*
**/
void Init_TOF400C(void);

/**
*
* @brief    获取TOF400C模块测量距离
* @param    void
* @return   unsigned int            测量出的距离,单位(mm)
* @notes
* Example:  Get_Distance_TOF400C();
*
**/
unsigned int Get_Distance_TOF400C(void);

// VL53L1X相关地址宏定义
#define VL53L1X_DEFAULT_DEVICE_ADDRESS                          0x0052
#define VL53L1_I2C_SLAVE__DEVICE_ADDRESS                        0x0001
#define GPIO_HV_MUX__CTRL                                       0x0030
#define GPIO__TIO_HV_STATUS                                     0x0031
#define RANGE_CONFIG__TIMEOUT_MACROP_A_HI                       0x005E
#define RANGE_CONFIG__TIMEOUT_MACROP_B_HI                       0x0061
#define SYSTEM__INTERRUPT_CLEAR                                 0x0086
#define RESULT__RANGE_STATUS                                    0x0089
#define VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0   0x0096

#endif /* __DMX_TOF400C_H */
