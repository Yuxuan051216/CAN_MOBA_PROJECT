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
 * @file       dmx_icm42688.c
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

#include "dmx_icm42688.h"
#include "dmx_delay.h"
#include "dmx_gpio.h"
#include "dmx_hard_spi.h"
#include "dmx_soft_spi.h"

#include "debug.h"
#include "dmx_oled.h"
#include "dmx_ips.h"
#include "dmx_tft180.h"

// ICM42688加速度计数据
float icm42688_acc_x  = 0, icm42688_acc_y  = 0, icm42688_acc_z  = 0;
// ICM42688角加速度数据
float icm42688_gyro_x = 0, icm42688_gyro_y = 0, icm42688_gyro_z = 0;

// SPI协议读写操作宏定义
#if ICM42688_HARD_SPI
#define ICM42688_Write_Reg(reg, data)       write_8bit_reg_hard_spi(ICM42688_SPI, reg, data);
#define ICM42688_Read_Regs(reg, data,num)   read_8bit_regs_hard_spi(ICM42688_SPI, reg | 0x80, data, num);
#else
static SOFT_SPI_struct ICM42688_SPI;
#define ICM42688_Write_Reg(reg, data)       write_8bitreg_soft_spi(&ICM42688_SPI, reg, data);
#define ICM42688_Read_Regs(reg, data,num)   read_8bitregs_soft_spi(&ICM42688_SPI, reg | 0x80, data, num);
#endif

// 静态函数声明,以下函数均为该.c文件内部调用
static void Write_Data_ICM42688(unsigned char reg, unsigned char data);
static void Read_Datas_ICM42688(unsigned char reg, unsigned char *data, unsigned int num);
// 数据转换为实际物理数据的转换系数
static float icm42688_acc_inv = 1, icm42688_gyro_inv = 1;

/**
*
* @brief    ICM42688陀螺仪初始化
* @param
* @return   void
* @notes    用户调用
* Example:  Init_ICM42688();
*
**/
void Init_ICM42688(void)
{
    // SPI初始化
#if ICM42688_HARD_SPI
    init_hard_spi(ICM42688_SPI, SPI_MODE0, ICM42688_SPI_SPEED, ICM42688_SPC_PIN, ICM42688_SDI_PIN, ICM42688_SDO_PIN);
#else
    init_soft_spi (&ICM42688_SPI, 0, ICM42688_SPC_PIN, ICM42688_SDI_PIN, ICM42688_SDO_PIN);
#endif
    init_gpio( ICM42688_CS_PIN  , GPO_PP , Speed_50MHZ , 1);

    // 初始化超时
    char time = 50;
    // 读取陀螺仪型号陀螺仪自检
    unsigned char model = 0xff;
    while(1)
    {
        // 读芯片ID
        Read_Datas_ICM42688(ICM42688_WHO_AM_I, &model, 1);
        if(model == 0x47)
        {
            // ICM42688,71
            break;
        }
        else
        {
            ICM42688_DELAY_MS(10);
            time--;
            if(time < 0)
            {
                switch(output_device_flag)
                {
                    case 0:printf("ICM42688 Init Error!\r\n");break;
                    case 1:Show_String_OLED(0,0,"ICM42688 Init Error!",Show6x8);break;
                    case 2:Show_String_IPS(0,0,"ICM42688 Init Error!",BLACK,WHITE,Show8x16);break;
                    case 3:Show_String_TFT180(0,0,"ICM42688 Init Error!",BLACK,WHITE,Show8x16);break;
                    default:printf("ICM42688 Init Error!\r\n");break;
                }
                while(1);
                // 卡在这里原因有以下几点
                // ICM42688坏了,如果是新的概率极低
                // 接线错误或者没有接好
                // 接线太长,通信失败
            }
        }
    }
    Write_Data_ICM42688(ICM42688_PWR_MGMT0, 0x00);      // 复位设备
    ICM42688_DELAY_MS(10);                              // 操作完PWR—MGMT0寄存器后200us内不能有任何读写寄存器的操作

    // 设置ICM42688加速度计和陀螺仪的量程和输出速率
    Set_LowpassFilter_Range_ICM42688(ICM42688_AFS_16G, ICM42688_AODR_32000HZ, ICM42688_GFS_2000DPS, ICM42688_GODR_32000HZ);

    Write_Data_ICM42688(ICM42688_PWR_MGMT0, 0x0f);      // 设置GYRO_MODE,ACCEL_MODE为低噪声模式
    ICM42688_DELAY_MS(10);
}

/**
*
* @brief    获得ICM42688陀螺仪加速度
* @param
* @return   void
* @notes    单位:g(m/s^2),用户调用
* Example:  Get_Acc_ICM42688();
*
**/
void Get_Acc_ICM42688(void)
{
    unsigned char data[6];
    Read_Datas_ICM42688(ICM42688_ACCEL_DATA_X1, data, 6);
    icm42688_acc_x = icm42688_acc_inv * (short int)(((short int)data[0] << 8) | data[1]);
    icm42688_acc_y = icm42688_acc_inv * (short int)(((short int)data[2] << 8) | data[3]);
    icm42688_acc_z = icm42688_acc_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

/**
*
* @brief    获得ICM42688陀螺仪角加速度
* @param
* @return   void
* @notes    单位为:°/s,用户调用
* Example:  Get_Gyro_ICM42688();
*
**/
void Get_Gyro_ICM42688(void)
{
    unsigned char data[6];
    Read_Datas_ICM42688(ICM42688_GYRO_DATA_X1, data, 6);
    icm42688_gyro_x = icm42688_gyro_inv * (short int)(((short int)data[0] << 8) | data[1]);
    icm42688_gyro_y = icm42688_gyro_inv * (short int)(((short int)data[2] << 8) | data[3]);
    icm42688_gyro_z = icm42688_gyro_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

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
void Set_LowpassFilter_Range_ICM42688(enum icm42688_afs afs, enum icm42688_aodr aodr, enum icm42688_gfs gfs, enum icm42688_godr godr)
{
    Write_Data_ICM42688(ICM42688_ACCEL_CONFIG0, (afs << 5) | (aodr + 1));   // 初始化ACCEL量程和输出速率(p77)
    Write_Data_ICM42688(ICM42688_GYRO_CONFIG0, (gfs << 5) | (godr + 1));    // 初始化GYRO量程和输出速率(p76)

    switch(afs)
    {
    case ICM42688_AFS_2G:
        icm42688_acc_inv = 2000 / 32768.0f;             // 加速度计量程为:±2g
        break;
    case ICM42688_AFS_4G:
        icm42688_acc_inv = 4000 / 32768.0f;             // 加速度计量程为:±4g
        break;
    case ICM42688_AFS_8G:
        icm42688_acc_inv = 8000 / 32768.0f;             // 加速度计量程为:±8g
        break;
    case ICM42688_AFS_16G:
        icm42688_acc_inv = 16000 / 32768.0f;            // 加速度计量程为:±16g
        break;
    default:
        icm42688_acc_inv = 1;                           // 不转化为实际数据
        break;
    }
    switch(gfs)
    {
    case ICM42688_GFS_15_625DPS:
        icm42688_gyro_inv = 15.625f / 32768.0f;         // 陀螺仪量程为:±15.625dps
        break;
    case ICM42688_GFS_31_25DPS:
        icm42688_gyro_inv = 31.25f / 32768.0f;          // 陀螺仪量程为:±31.25dps
        break;
    case ICM42688_GFS_62_5DPS:
        icm42688_gyro_inv = 62.5f / 32768.0f;           // 陀螺仪量程为:±62.5dps
        break;
    case ICM42688_GFS_125DPS:
        icm42688_gyro_inv = 125.0f / 32768.0f;          // 陀螺仪量程为:±125dps
        break;
    case ICM42688_GFS_250DPS:
        icm42688_gyro_inv = 250.0f / 32768.0f;          // 陀螺仪量程为:±250dps
        break;
    case ICM42688_GFS_500DPS:
        icm42688_gyro_inv = 500.0f / 32768.0f;          // 陀螺仪量程为:±500dps
        break;
    case ICM42688_GFS_1000DPS:
        icm42688_gyro_inv = 1000.0f / 32768.0f;         // 陀螺仪量程为:±1000dps
        break;
    case ICM42688_GFS_2000DPS:
        icm42688_gyro_inv = 2000.0f / 32768.0f;         // 陀螺仪量程为:±2000dps
        break;
    default:
        icm42688_gyro_inv = 1;                          // 不转化为实际数据
        break;
    }
}

/**
*
* @brief    ICM42688陀螺仪写数据
* @param    reg                 寄存器
* @param    data                需要写进该寄存器的数据
* @return   void
* @notes    ICM42688.c文件内部调用,用户无需调用尝试
* Example:  Write_Data_ICM42688(0x00,0x00);
*
**/
static void Write_Data_ICM42688(unsigned char reg, unsigned char data)
{
    ICM42688_CS_LEVEL(0);
    ICM42688_Write_Reg(reg, data);
    ICM42688_CS_LEVEL(1);
}

/**
*
* @brief    ICM42688陀螺仪读数据
* @param    reg                 寄存器
* @param    data                把读出的数据存入data
* @param    num                 数据个数
* @return   void
* @notes    ICM42688.c文件内部调用,用户无需调用尝试
* Example:  Read_Datas_ICM42688(0x00,data,1);
*
**/
static void Read_Datas_ICM42688(unsigned char reg, unsigned char *data, unsigned int num)
{
    ICM42688_CS_LEVEL(0);
    ICM42688_Read_Regs(reg, data, num);
    ICM42688_CS_LEVEL(1);
}
