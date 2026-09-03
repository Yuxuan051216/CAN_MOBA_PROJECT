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
 * @file       dmx_icm42688.h
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

#ifndef __DMX_ICM42688_H
#define __DMX_ICM42688_H

// 通信方式选择,0:软件SPI,1:硬件SPI
#define ICM42688_HARD_SPI       1

// ICM42688管脚宏定义
#if ICM42688_HARD_SPI
#define ICM42688_SPI_SPEED      (24*1000*1000)      // 硬件SPI速率
#define ICM42688_SPI            (SPI_3)             // 硬件SPI号
#define ICM42688_SPC_PIN        (SPI3_RM0_SCK_B3)   // 硬件SPI_SCK引脚
#define ICM42688_SDI_PIN        (SPI3_RM0_MOSI_B5)  // 硬件SPI_MOSI引脚
#define ICM42688_SDO_PIN        (SPI3_RM0_MISO_B4)  // 硬件SPI_MISO引脚
#else
// ICM42688,SPC管脚对应CH32V307VCT6引脚B3
#define ICM42688_SPC_PIN        B3
// ICM42688,SD1管脚对应CH32V307VCT6引脚B5
#define ICM42688_SDI_PIN        B5
// ICM42688,SD0管脚对应CH32V307VCT6引脚B4
#define ICM42688_SDO_PIN        B4
#endif
// ICM42688,CS管脚对应CH32V307VCT6引脚C10
#define ICM42688_CS_PIN         C10

// ICM42688延时
#define ICM42688_DELAY_MS(time)  (Delay_ms(time))
// 对ICM42688的CS管脚进行高低电平操作
#define ICM42688_CS_LEVEL(level) set_level_gpio(ICM42688_CS_PIN  , level);

// 声明ICM42688加速度计数据
extern float icm42688_acc_x  , icm42688_acc_y  , icm42688_acc_z  ;
// 声明ICM42688角加速度数据
extern float icm42688_gyro_x , icm42688_gyro_y , icm42688_gyro_z ;

enum icm42688_afs
{
    ICM42688_AFS_16G,   // default
    ICM42688_AFS_8G,
    ICM42688_AFS_4G,
    ICM42688_AFS_2G,
    NUM_ICM42688__AFS
};
enum icm42688_aodr
{
    ICM42688_AODR_32000HZ,
    ICM42688_AODR_16000HZ,
    ICM42688_AODR_8000HZ,
    ICM42688_AODR_4000HZ,
    ICM42688_AODR_2000HZ,
    ICM42688_AODR_1000HZ,// default
    ICM42688_AODR_200HZ,
    ICM42688_AODR_100HZ,
    ICM42688_AODR_50HZ,
    ICM42688_AODR_25HZ,
    ICM42688_AODR_12_5HZ,
    ICM42688_AODR_6_25HZ,
    ICM42688_AODR_3_125HZ,
    ICM42688_AODR_1_5625HZ,
    ICM42688_AODR_500HZ,
    NUM_ICM42688_AODR
};

enum icm42688_gfs
{
    ICM42688_GFS_2000DPS,// default
    ICM42688_GFS_1000DPS,
    ICM42688_GFS_500DPS,
    ICM42688_GFS_250DPS,
    ICM42688_GFS_125DPS,
    ICM42688_GFS_62_5DPS,
    ICM42688_GFS_31_25DPS,
    ICM42688_GFS_15_625DPS,
    NUM_ICM42688_GFS
};
enum icm42688_godr
{
    ICM42688_GODR_32000HZ,
    ICM42688_GODR_16000HZ,
    ICM42688_GODR_8000HZ,
    ICM42688_GODR_4000HZ,
    ICM42688_GODR_2000HZ,
    ICM42688_GODR_1000HZ,// default
    ICM42688_GODR_200HZ,
    ICM42688_GODR_100HZ,
    ICM42688_GODR_50HZ,
    ICM42688_GODR_25HZ,
    ICM42688_GODR_12_5HZ,
    ICM42688_GODR_X0HZ,
    ICM42688_GODR_X1HZ,
    ICM42688_GODR_X2HZ,
    ICM42688_GODR_500HZ,
    NUM_ICM42688_GODR
};

/**
*
* @brief    ICM42688陀螺仪初始化
* @param
* @return   void
* @notes    用户调用
* Example:  Init_ICM42688();
*
**/
void Init_ICM42688(void);

/**
*
* @brief    获得ICM42688陀螺仪加速度
* @param
* @return   void
* @notes    单位:g(m/s^2),用户调用
* Example:  Get_Acc_ICM42688();
*
**/
void Get_Acc_ICM42688(void);

/**
*
* @brief    获得ICM42688陀螺仪角加速度
* @param
* @return   void
* @notes    单位为:°/s,用户调用
* Example:  Get_Gyro_ICM42688();
*
**/
void Get_Gyro_ICM42688(void);

/**
*
* @brief    设置ICM42688陀螺仪低通滤波器带宽和量程
* @param    afs                 // 加速度计量程,可在dmx_icm42688.h文件里枚举定义中查看
* @param    aodr                // 加速度计输出速率,可在dmx_icm42688.h文件里枚举定义中查看
* @param    gfs                 // 陀螺仪量程,可在dmx_icm42688.h文件里枚举定义中查看
* @param    godr                // 陀螺仪输出速率,可在dmx_icm42688.h文件里枚举定义中查看
* @return   void
* @notes    ICM42688.c文件内部调用,用户无需调用尝试
* Example:  Set_LowpassFilter_Range_ICM42688(ICM42688_AFS_16G,ICM42688_AODR_32000HZ,ICM42688_GFS_2000DPS,ICM42688_GODR_32000HZ);
*
**/
void Set_LowpassFilter_Range_ICM42688(enum icm42688_afs afs, enum icm42688_aodr aodr, enum icm42688_gfs gfs, enum icm42688_godr godr);

// ICM42688Bank0内部地址
#define ICM42688_DEVICE_CONFIG             0x11
#define ICM42688_DRIVE_CONFIG              0x13
#define ICM42688_INT_CONFIG                0x14
#define ICM42688_FIFO_CONFIG               0x16
#define ICM42688_TEMP_DATA1                0x1D
#define ICM42688_TEMP_DATA0                0x1E
#define ICM42688_ACCEL_DATA_X1             0x1F
#define ICM42688_ACCEL_DATA_X0             0x20
#define ICM42688_ACCEL_DATA_Y1             0x21
#define ICM42688_ACCEL_DATA_Y0             0x22
#define ICM42688_ACCEL_DATA_Z1             0x23
#define ICM42688_ACCEL_DATA_Z0             0x24
#define ICM42688_GYRO_DATA_X1              0x25
#define ICM42688_GYRO_DATA_X0              0x26
#define ICM42688_GYRO_DATA_Y1              0x27
#define ICM42688_GYRO_DATA_Y0              0x28
#define ICM42688_GYRO_DATA_Z1              0x29
#define ICM42688_GYRO_DATA_Z0              0x2A
#define ICM42688_TMST_FSYNCH               0x2B
#define ICM42688_TMST_FSYNCL               0x2C
#define ICM42688_INT_STATUS                0x2D
#define ICM42688_FIFO_COUNTH               0x2E
#define ICM42688_FIFO_COUNTL               0x2F
#define ICM42688_FIFO_DATA                 0x30
#define ICM42688_APEX_DATA0                0x31
#define ICM42688_APEX_DATA1                0x32
#define ICM42688_APEX_DATA2                0x33
#define ICM42688_APEX_DATA3                0x34
#define ICM42688_APEX_DATA4                0x35
#define ICM42688_APEX_DATA5                0x36
#define ICM42688_INT_STATUS2               0x37
#define ICM42688_INT_STATUS3               0x38
#define ICM42688_SIGNAL_PATH_RESET         0x4B
#define ICM42688_INTF_CONFIG0              0x4C
#define ICM42688_INTF_CONFIG1              0x4D
#define ICM42688_PWR_MGMT0                 0x4E
#define ICM42688_GYRO_CONFIG0              0x4F
#define ICM42688_ACCEL_CONFIG0             0x50
#define ICM42688_GYRO_CONFIG1              0x51
#define ICM42688_GYRO_ACCEL_CONFIG0        0x52
#define ICM42688_ACCEL_CONFIG1             0x53
#define ICM42688_TMST_CONFIG               0x54
#define ICM42688_APEX_CONFIG0              0x56
#define ICM42688_SMD_CONFIG                0x57
#define ICM42688_FIFO_CONFIG1              0x5F
#define ICM42688_FIFO_CONFIG2              0x60
#define ICM42688_FIFO_CONFIG3              0x61
#define ICM42688_FSYNC_CONFIG              0x62
#define ICM42688_INT_CONFIG0               0x63
#define ICM42688_INT_CONFIG1               0x64
#define ICM42688_INT_SOURCE0               0x65
#define ICM42688_INT_SOURCE1               0x66
#define ICM42688_INT_SOURCE3               0x68
#define ICM42688_INT_SOURCE4               0x69
#define ICM42688_FIFO_LOST_PKT0            0x6C
#define ICM42688_FIFO_LOST_PKT1            0x6D
#define ICM42688_SELF_TEST_CONFIG          0x70
#define ICM42688_WHO_AM_I                  0x75
#define ICM42688_REG_BANK_SEL              0x76// Banks
#define ICM42688_SENSOR_CONFIG0            0x03
#define ICM42688_GYRO_CONFIG_STATIC2       0x0B
#define ICM42688_GYRO_CONFIG_STATIC3       0x0C
#define ICM42688_GYRO_CONFIG_STATIC4       0x0D
#define ICM42688_GYRO_CONFIG_STATIC5       0x0E
#define ICM42688_GYRO_CONFIG_STATIC6       0x0F
#define ICM42688_GYRO_CONFIG_STATIC7       0x10
#define ICM42688_GYRO_CONFIG_STATIC8       0x11
#define ICM42688_GYRO_CONFIG_STATIC9       0x12
#define ICM42688_GYRO_CONFIG_STATIC10      0x13
#define ICM42688_XG_ST_DATA                0x5F
#define ICM42688_YG_ST_DATA                0x60
#define ICM42688_ZG_ST_DATA                0x61
#define ICM42688_TMSTVAL0                  0x62
#define ICM42688_TMSTVAL1                  0x63
#define ICM42688_TMSTVAL2                  0x64
#define ICM42688_INTF_CONFIG4              0x7A
#define ICM42688_INTF_CONFIG5              0x7B
#define ICM42688_INTF_CONFIG6              0x7C

#endif /* __DMX_ICM42688_H */
