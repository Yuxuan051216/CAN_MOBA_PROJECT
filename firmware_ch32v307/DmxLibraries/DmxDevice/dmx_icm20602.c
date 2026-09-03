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
 * @file       dmx_icm20602.c
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

#include "dmx_icm20602.h"
#include "dmx_delay.h"
#include "dmx_gpio.h"
#include "dmx_hard_spi.h"
#include "dmx_soft_spi.h"

#include "debug.h"
#include "dmx_oled.h"
#include "dmx_ips.h"
#include "dmx_tft180.h"

// ICM20602加速度计数据
float icm20602_acc_x  = 0, icm20602_acc_y  = 0, icm20602_acc_z  = 0;
// ICM20602角加速度数据
float icm20602_gyro_x = 0, icm20602_gyro_y = 0, icm20602_gyro_z = 0;

// SPI协议读写操作宏定义
#if ICM20602_HARD_SPI
#define ICM20602_Write_Reg(reg, data)       write_8bit_reg_hard_spi(ICM20602_SPI, reg, data);
#define ICM20602_Read_Regs(reg, data,num)   read_8bit_regs_hard_spi(ICM20602_SPI, reg | 0x80, data, num);
#else
static SOFT_SPI_struct ICM20602_SPI;
#define ICM20602_Write_Reg(reg, data)       write_8bitreg_soft_spi(&ICM20602_SPI, reg, data);
#define ICM20602_Read_Regs(reg, data,num)   read_8bitregs_soft_spi(&ICM20602_SPI, reg | 0x80, data, num);
#endif

// 静态函数声明,以下函数均为该.c文件内部调用
static void Write_Data_ICM20602(unsigned char reg, unsigned char data);
static void Read_Datas_ICM20602(unsigned char reg, unsigned char *data, unsigned int num);
// 数据转换为实际物理数据的转换系数
static float icm20602_acc_inv = 1, icm20602_gyro_inv = 1;

/**
*
* @brief    ICM20602陀螺仪初始化
* @param
* @return   void
* @notes    用户调用
* Example:  Init_ICM20602();
*
**/
void Init_ICM20602(void)
{
    // SPI初始化
#if ICM20602_HARD_SPI
    init_hard_spi(ICM20602_SPI, SPI_MODE0, ICM20602_SPI_SPEED, ICM20602_SPC_PIN, ICM20602_SDI_PIN, ICM20602_SDO_PIN);
#else
    init_soft_spi (&ICM20602_SPI, 0, ICM20602_SPC_PIN, ICM20602_SDI_PIN, ICM20602_SDO_PIN);
#endif
    init_gpio( ICM20602_CS_PIN  , GPO_PP , Speed_50MHZ , 1);

    // 初始化超时
    char time = 50;
    // 读取陀螺仪型号陀螺仪自检
    unsigned char model = 0xff;
    while(1)
    {
        Read_Datas_ICM20602(WHO_AM_I, &model, 1);
        if(model == 0x12)
        {
            // ICM20602
            break;
        }
        else
        {
            ICM20602_DELAY_MS(10);
            time--;
            if(time < 0)
            {
                switch(output_device_flag)
                {
                    case 0:printf("ICM20602 Init Error!\r\n");break;
                    case 1:Show_String_OLED(0,0,"ICM20602 Init Error!",Show6x8);break;
                    case 2:Show_String_IPS(0,0,"ICM20602 Init Error!",BLACK,WHITE,Show8x16);break;
                    case 3:Show_String_TFT180(0,0,"ICM20602 Init Error!",BLACK,WHITE,Show8x16);break;
                    default:printf("ICM20602 Init Error!\r\n");break;
                }
                while(1);
                // 卡在这里原因有以下几点
                // ICM20602坏了,如果是新的概率极低
                // 接线错误或者没有接好
            }
        }
    }
    Write_Data_ICM20602(PWR_MGMT_1, 0x80); // 复位设备
    ICM20602_DELAY_MS(10);
    Write_Data_ICM20602(PWR_MGMT_1, 0x01);
    ICM20602_DELAY_MS(10);
    Write_Data_ICM20602(PWR_MGMT_2, 0x00);
    ICM20602_DELAY_MS(10);

    Write_Data_ICM20602(SMPLRT_DIV, 0x07);

    // 设置ICM20602陀螺仪低通滤波器带宽和量程,ICM20602和MPU6050均为同一家产品前者可看作后者的升级款
    Set_LowpassFilter_Range_ICM20602(MPU_ABW_218, MPU_GBW_176, MPU_FS_4G, MPU_FS_2000DPS);
}

/**
*
* @brief    获得ICM20602陀螺仪加速度
* @param
* @return   void
* @notes    单位:g(m/s^2),用户调用
* Example:  Get_Acc_ICM20602();
*
**/
void Get_Acc_ICM20602(void)
{
    unsigned char data[6];
    Read_Datas_ICM20602(ACCEL_XOUT_H, data, 6);
    icm20602_acc_x = icm20602_acc_inv * (short int)(((short int)data[0] << 8) | data[1]);
    icm20602_acc_y = icm20602_acc_inv * (short int)(((short int)data[2] << 8) | data[3]);
    icm20602_acc_z = icm20602_acc_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

/**
*
* @brief    获得ICM20602陀螺仪角加速度
* @param
* @return   void
* @notes    单位为:°/s,用户调用
* Example:  Get_Gyro_ICM20602();
*
**/
void Get_Gyro_ICM20602(void)
{
    unsigned char data[6];
    Read_Datas_ICM20602(GYRO_XOUT_H, data, 6);
    icm20602_gyro_x = icm20602_gyro_inv * (short int)(((short int)data[0] << 8) | data[1]);
    icm20602_gyro_y = icm20602_gyro_inv * (short int)(((short int)data[2] << 8) | data[3]);
    icm20602_gyro_z = icm20602_gyro_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

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
void Set_LowpassFilter_Range_ICM20602(enum mpu_acc_bw abw, enum mpu_gyro_bw gbw, enum mpu_acc_fs afs, enum mpu_gyro_fs gfs)
{
    Write_Data_ICM20602(MPU6050_CONFIG, gbw);
    Write_Data_ICM20602(ACCEL_CONFIG_2, abw);
    Write_Data_ICM20602(GYRO_CONFIG, gfs << 3);
    Write_Data_ICM20602(ACCEL_CONFIG, afs << 3);
    switch(afs)
    {
    case MPU_FS_2G:
        icm20602_acc_inv = (float)2.0 * 9.8 / 32768.0;   // 加速度计量程为:±2g
        break;
    case MPU_FS_4G:
        icm20602_acc_inv = (float)4.0 * 9.8 / 32768.0;   // 加速度计量程为:±4g
        break;
    case MPU_FS_8G:
        icm20602_acc_inv = (float)8.0 * 9.8 / 32768.0;   // 加速度计量程为:±8g
        break;
    case MPU_FS_16G:
        icm20602_acc_inv = (float)16.0 * 9.8 / 32768.0;  // 加速度计量程为:±16g
        break;
    default:
        icm20602_acc_inv = 1;                            // 不转化为实际数据
        break;
    }
    switch(gfs)
    {
    case MPU_FS_250DPS:
        icm20602_gyro_inv = (float)250.0 / 32768.0;      // 陀螺仪量程为:±250dps
        break;
    case MPU_FS_500DPS:
        icm20602_gyro_inv = (float)500.0 / 32768.0;      // 陀螺仪量程为:±500dps
        break;
    case MPU_FS_1000DPS:
        icm20602_gyro_inv = (float)1000.0 / 32768.0;     // 陀螺仪量程为:±1000dps
        break;
    case MPU_FS_2000DPS:
        icm20602_gyro_inv = (float)2000.0 / 32768.0;     // 陀螺仪量程为:±2000dps
        break;
    default:
        icm20602_gyro_inv = 1;                           // 不转化为实际数据
        break;
    }
}

/**
*
* @brief    ICM20602陀螺仪写数据
* @param    reg                 寄存器
* @param    data                需要写进该寄存器的数据
* @return   void
* @notes    ICM20602.c文件内部调用,用户无需调用尝试
* Example:  Write_Data_ICM20602(0x00,0x00);
*
**/
static void Write_Data_ICM20602(unsigned char reg, unsigned char data)
{
    ICM20602_CS_LEVEL(0);
    ICM20602_Write_Reg(reg, data);
    ICM20602_CS_LEVEL(1);
}

/**
*
* @brief    ICM20602陀螺仪读数据
* @param    reg                 寄存器
* @param    data                把读出的数据存入data
* @param    num                 数据个数
* @return   void
* @notes    ICM20602.c文件内部调用,用户无需调用尝试
* Example:  Read_Datas_ICM20602(0x00,data,1);
*
**/
static void Read_Datas_ICM20602(unsigned char reg, unsigned char *data, unsigned int num)
{
    ICM20602_CS_LEVEL(0);
    ICM20602_Read_Regs(reg, data, num);
    ICM20602_CS_LEVEL(1);
}
