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
 * @file       dmx_icm20602.h
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

#ifndef __DMX_ICM20602_H
#define __DMX_ICM20602_H

#include "dmx_mpu.h"

// 通信方式选择,0:软件SPI,1:硬件SPI
#define ICM20602_HARD_SPI       1

// IPS屏幕管脚宏定义
#if ICM20602_HARD_SPI
#define ICM20602_SPI_SPEED      (10*1000*1000)      // 硬件SPI速率
#define ICM20602_SPI            (SPI_3)             // 硬件SPI号
#define ICM20602_SPC_PIN        (SPI3_RM0_SCK_B3)   // 硬件SPI_SCK引脚
#define ICM20602_SDI_PIN        (SPI3_RM0_MOSI_B5)  // 硬件SPI_MOSI引脚
#define ICM20602_SDO_PIN        (SPI3_RM0_MISO_B4)  // 硬件SPI_MISO引脚
#else
// ICM20602,SPC管脚对应CH32V307VCT6引脚B3
#define ICM20602_SPC_PIN        B3
// ICM20602,SD1管脚对应CH32V307VCT6引脚B5
#define ICM20602_SDI_PIN        B5
// ICM20602,SD0管脚对应CH32V307VCT6引脚B4
#define ICM20602_SDO_PIN        B4
#endif
// ICM20602,CS管脚对应CH32V307VCT6引脚C10
#define ICM20602_CS_PIN         C10

// ICM20602延时
#define ICM20602_DELAY_MS(time)  (Delay_ms(time))
// 对ICM20602屏幕CS管脚进行高低电平操作
#define ICM20602_CS_LEVEL(level) set_level_gpio(ICM20602_CS_PIN  , level);

// 声明ICM20602加速度计数据
extern float icm20602_acc_x , icm20602_acc_y , icm20602_acc_z ;
// 声明ICM20602角加速度数据
extern float icm20602_gyro_x, icm20602_gyro_y, icm20602_gyro_z;

/**
*
* @brief    ICM20602陀螺仪初始化
* @param
* @return   void
* @notes    用户调用
* Example:  Init_ICM20602();
*
**/
void Init_ICM20602(void);

/**
*
* @brief    获得ICM20602陀螺仪加速度
* @param
* @return   void
* @notes    单位:g(m/s^2),用户调用
* Example:  Get_Acc_ICM20602();
*
**/
void Get_Acc_ICM20602(void);

/**
*
* @brief    获得ICM20602陀螺仪角加速度
* @param
* @return   void
* @notes    单位为:°/s,用户调用
* Example:  Get_Gyro_ICM20602();
*
**/
void Get_Gyro_ICM20602(void);

/**
*
* @brief    设置ICM20602陀螺仪低通滤波器带宽和量程
* @param    abw                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    gbw                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    afs                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    gfs                 // 可在dmx_mpu.h文件里枚举定义中查看
* @return   void
* @notes    ICM20602.c文件内部调用,用户无需调用尝试
* Example:  Set_LowpassFilter_Range_ICM20602(MPU_ABW_218, MPU_GBW_176,MPU_FS_4G, MPU_FS_2000DPS);
*
**/
void Set_LowpassFilter_Range_ICM20602(enum mpu_acc_bw abw, enum mpu_gyro_bw gbw, enum mpu_acc_fs afs, enum mpu_gyro_fs gfs);

#endif /* __DMX_ICM20602_H */
