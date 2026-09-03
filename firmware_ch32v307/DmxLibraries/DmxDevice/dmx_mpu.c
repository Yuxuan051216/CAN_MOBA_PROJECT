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
 * @file       dmx_mpu.c
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

#include "dmx_mpu.h"
#include "dmx_delay.h"
#include "dmx_gpio.h"

#include "debug.h"
#include "dmx_oled.h"
#include "dmx_ips.h"
#include "dmx_tft180.h"

// MPU6050加速度计数据
float mpu_acc_x  = 0, mpu_acc_y  = 0, mpu_acc_z  = 0;
// MPU6050陀螺仪数据
float mpu_gyro_x = 0, mpu_gyro_y = 0, mpu_gyro_z = 0;
// 静态函数声明,以下函数均为该.c文件内部调用
static void MPU_Write_Data(unsigned char addr, unsigned char reg, unsigned char dat);
static void MPU_Read_Datas(unsigned char addr, unsigned char reg, unsigned char *dat, unsigned char num);
// 数据转换为实际物理数据的转换系数
static float mpu_acc_inv = 1, mpu_gyro_inv = 1;
// 陀螺仪地址
static unsigned char GYROSCOPE_DEV_ADDR;

/**
*
* @brief    MPU6050陀螺仪初始化
* @param
* @return   void
* @notes    用户调用
* Example:  Init_MPU();
*
**/
void Init_MPU(void)
{
    init_gpio( MPU_SCL_PIN , GPO_OD , Speed_50MHZ , 1);
    init_gpio( MPU_SDA_PIN , GPO_OD , Speed_50MHZ , 1);
    // 初始化超时
    char time = 50;
    // 读取陀螺仪型号陀螺仪自检
    unsigned char model = 0xff;
    unsigned char slave_addr = 0x68;
    while(1)
    {
        MPU_Read_Datas(slave_addr, WHO_AM_I, &model, 1);
        if(model == 0x68)
        {
            // MPU6050,104
            GYROSCOPE_DEV_ADDR = 0x68;
            break;
        }
        else if(model == 0x12)
        {
            // ICM20602
            GYROSCOPE_DEV_ADDR = 0x69;
            break;
        }
        else
        {
            slave_addr = slave_addr ^ 0x01; // 反转陀螺仪地址最低位(0x69,0x68)
            MPU_DELAY_MS(10);
            time--;
            if(time < 0)
            {
                switch(output_device_flag)
                {
                    case 0:printf("MPU Init Error!\r\n");break;
                    case 1:Show_String_OLED(0,0,"MPU Init Error!",Show6x8);break;
                    case 2:Show_String_IPS(0,0,"MPU Init Error!",BLACK,WHITE,Show8x16);break;
                    case 3:Show_String_TFT180(0,0,"MPU Init Error!",BLACK,WHITE,Show8x16);break;
                    default:printf("MPU Init Error!\r\n");break;
                }
                while(1);
                // 卡在这里原因有以下几点
                // MPU6050或者ICM20602坏了,如果是新的概率极低
                // 接线错误或者没有接好
                // 可能需要外接上拉电阻,上拉到3.3V
            }
        }
    }
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, PWR_MGMT_1, 0x80);
    MPU_DELAY_MS(10);

    MPU_Write_Data(GYROSCOPE_DEV_ADDR, PWR_MGMT_1, 0x00);                 // 解除休眠状态
    MPU_DELAY_MS(10);
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, PWR_MGMT_2, 0x00);
    MPU_DELAY_MS(10);

    MPU_Write_Data(GYROSCOPE_DEV_ADDR, SMPLRT_DIV, 0x07);                 // 125HZ采样率

    // 设置陀螺仪低通滤波器带宽和量程
    Set_LowpassFilter_Range_MPU(MPU_ABW_218, MPU_GBW_176, MPU_FS_4G, MPU_FS_2000DPS);

    MPU_Write_Data(GYROSCOPE_DEV_ADDR, USR_CONTROL, 0x00);                // 关闭MPU6050对辅助IIC设备的控制
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, INT_PIN_CFG, 0x02);
}

/**
*
* @brief    获得MPU6050陀螺仪加速度
* @param
* @return   void
* @notes    单位:g(m/s^2),用户调用
* Example:  Get_Acc_MPU();
*
**/
void Get_Acc_MPU(void)
{
    unsigned char data[6];
    MPU_Read_Datas(GYROSCOPE_DEV_ADDR, ACCEL_XOUT_H, data, 6);
    mpu_acc_x = mpu_acc_inv * (short int)(((short int)data[0] << 8) | data[1]);
    mpu_acc_y = mpu_acc_inv * (short int)(((short int)data[2] << 8) | data[3]);
    mpu_acc_z = mpu_acc_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

/**
*
* @brief    获得MPU6050陀螺仪角加速度
* @param
* @return   void
* @notes    单位为:°/s,用户调用
* Example:  Get_Gyro_MPU();
*
**/
void Get_Gyro_MPU(void)
{
    unsigned char data[6];
    MPU_Read_Datas(GYROSCOPE_DEV_ADDR, GYRO_XOUT_H, data, 6);
    mpu_gyro_x = mpu_gyro_inv * (short int)(((short int)data[0] << 8) | data[1]);
    mpu_gyro_y = mpu_gyro_inv * (short int)(((short int)data[2] << 8) | data[3]);
    mpu_gyro_z = mpu_gyro_inv * (short int)(((short int)data[4] << 8) | data[5]);
}

/**
*
* @brief    设置MPU6050陀螺仪低通滤波器带宽和量程
* @param    abw                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    gbw                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    afs                 // 可在dmx_mpu.h文件里枚举定义中查看
* @param    gfs                 // 可在dmx_mpu.h文件里枚举定义中查看
* @return   void
* @notes    dmx_mpu.c文件内部调用,用户无需调用尝试
* Example:  Set_LowpassFilter_Range_MPU(MPU_ABW_218, MPU_GBW_176,MPU_FS_4G, MPU_FS_2000DPS);
*
**/
void Set_LowpassFilter_Range_MPU(enum mpu_acc_bw abw, enum mpu_gyro_bw gbw, enum mpu_acc_fs afs, enum mpu_gyro_fs gfs)
{
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, MPU6050_CONFIG, gbw);
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, ACCEL_CONFIG_2, abw);
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, GYRO_CONFIG, gfs << 3);
    MPU_Write_Data(GYROSCOPE_DEV_ADDR, ACCEL_CONFIG, afs << 3);
    switch(afs)
    {
    case MPU_FS_2G:
        mpu_acc_inv = (float)2.0 * 9.8 / 32768.0;       // 加速度计量程为:±2g
        break;
    case MPU_FS_4G:
        mpu_acc_inv = (float)4.0 * 9.8 / 32768.0;       // 加速度计量程为:±4g
        break;
    case MPU_FS_8G:
        mpu_acc_inv = (float)8.0 * 9.8 / 32768.0;       // 加速度计量程为:±8g
        break;
    case MPU_FS_16G:
        mpu_acc_inv = (float)16.0 * 9.8 / 32768.0;      // 加速度计量程为:±16g
        break;
    default:
        mpu_acc_inv = 1;                                // 不转化为实际数据
        break;
    }
    switch(gfs)
    {
    case MPU_FS_250DPS:
        mpu_gyro_inv = (float)250.0 / 32768.0;          // 陀螺仪量程为:±250dps
        break;
    case MPU_FS_500DPS:
        mpu_gyro_inv = (float)500.0 / 32768.0;          // 陀螺仪量程为:±500dps
        break;
    case MPU_FS_1000DPS:
        mpu_gyro_inv = (float)1000.0 / 32768.0;         // 陀螺仪量程为:±1000dps
        break;
    case MPU_FS_2000DPS:
        mpu_gyro_inv = (float)2000.0 / 32768.0;         // 陀螺仪量程为:±2000dps
        break;
    default:
        mpu_gyro_inv = 1;                               // 不转化为实际数据
        break;
    }
}



/**
*
* @brief    MPU软件IIC开始
* @param    void
* @return   void
* @notes    内部调用
* Example:  MPU_Start();
*
**/
static void MPU_Start(void)
{
    MPU_SCL_LEVEL(1);
    MPU_SDA_LEVEL(1);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SDA_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SCL_LEVEL(0);
}

/**
*
* @brief    MPU软件IIC停止
* @param    void
* @return   void
* @notes    内部调用
* Example:  MPU_Stop();
*
**/
static void MPU_Stop(void)
{
    MPU_SCL_LEVEL(0);
    MPU_SDA_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SCL_LEVEL(1);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SDA_LEVEL(1);
    MPU_DELAY_MS(MPU_DELAY);
}

/**
*
* @brief    主机向从设备发送应答/非应答信号 1/0
* @param    ack             发送信号
* @return   void
* @notes    内部调用
* Example:  MPU_Sendack(1);
*
**/
static void MPU_Sendack(unsigned char ack)
{
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    if(ack)
        MPU_SDA_LEVEL(0);
    else
        MPU_SDA_LEVEL(1);
    MPU_SCL_LEVEL(1);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
}

/**
*
* @brief    主机等待从设备应答信号
* @param    void
* @return   int
* @notes    内部调用
* Example:  MPU_Waitack();
*
**/
static int MPU_Waitack(void)
{
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SCL_LEVEL(1);
    MPU_DELAY_MS(MPU_DELAY);
    if(get_level_gpio(MPU_SDA_PIN))
    {
        MPU_SCL_LEVEL(1);
        return 0;
    }
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    return 1;
}

/**
*
* @brief    发送单个字节
* @param    ch                  字节
* @return   void
* @notes    内部调用
* Example:  MPU_Write_Char(0X66);
*
**/
static void MPU_Write_Char(unsigned char ch)
{
    unsigned char i = 8;
    while(i--)
    {
        if(ch & 0x80)
            MPU_SDA_LEVEL(1);
        else
            MPU_SDA_LEVEL(0);
        ch <<= 1;
        MPU_DELAY_MS(MPU_DELAY);
        MPU_SCL_LEVEL(1);
        MPU_DELAY_MS(MPU_DELAY);
        MPU_SCL_LEVEL(0);
    }
    MPU_Waitack();
}

/**
*
* @brief    接受一个字节数据
* @param    ack                 应答信号
* @return   unsigned char       接受字节
* @notes    内部调用
* Example:  MPU_Read_Char(0);
*
**/
static unsigned char MPU_Read_Char(unsigned char ack)
{
    unsigned char i;
    unsigned char c = 0;
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_SDA_LEVEL(1);
    for(i = 0; i < 8; i++)
    {
        MPU_DELAY_MS(MPU_DELAY);
        MPU_SCL_LEVEL(0);
        MPU_DELAY_MS(MPU_DELAY);
        MPU_SCL_LEVEL(1);
        MPU_DELAY_MS(MPU_DELAY);
        c <<= 1;
        if(get_level_gpio(MPU_SDA_PIN))
        {
            c += 1;
        }
    }
    MPU_SCL_LEVEL(0);
    MPU_DELAY_MS(MPU_DELAY);
    MPU_Sendack(ack);
    return c;
}

/**
*
* @brief    写数据到设备寄存器中
* @param    addr                设备地址
* @param    reg                 设备寄存器地址
* @param    dat                 数据地址
* @return   void
* @notes        内部调用
* Example:  MPU_Write_Data(addr,reg,data)
*
**/
static void MPU_Write_Data(unsigned char addr, unsigned char reg, unsigned char dat)
{
    MPU_Start();
    MPU_Write_Char(addr << 1);
    MPU_Write_Char(reg);
    MPU_Write_Char(dat);
    MPU_Stop();
}

/**
*
* @brief    从设备寄存器读取多字节数据
* @param    addr                设备地址
* @param    reg                 设备寄存器地址
* @param    dat                 数据地址
* @param    num             数据长度
* @return   void
* @notes        内部调用
* Example:  MPU_Read_Datas(addr,reg,data,num)
*
**/
static void MPU_Read_Datas(unsigned char addr, unsigned char reg, unsigned char *dat, unsigned char num)
{
    MPU_Start();
    MPU_Write_Char(addr << 1 | 0x00);
    MPU_Write_Char(reg);
    MPU_Start();
    MPU_Write_Char((addr << 1) | 0x01);
    while(--num)
    {
        *dat = MPU_Read_Char(1);
        dat++;
    }
    *dat = MPU_Read_Char(0);
    MPU_Stop();
}
