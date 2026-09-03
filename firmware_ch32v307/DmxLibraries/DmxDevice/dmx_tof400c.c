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
 * @file       dmx_tof400c.c
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

#include "dmx_tof400c.h"
#include "dmx_delay.h"
#include "dmx_gpio.h"

#include "debug.h"
#include "dmx_oled.h"
#include "dmx_ips.h"
#include "dmx_tft180.h"

// 静态函数声明,以下函数均为该.c文件内部调用
static void TOF400C_Write_Datas(unsigned int reg, unsigned char *dat, unsigned long num);
static void TOF400C_Read_Datas(unsigned int reg, unsigned char *dat, unsigned long num);
static void TOF400C_Write_Data(unsigned char reg, unsigned char dat);
static void TOF400C_Read_Data(unsigned char  reg, unsigned char *dat);
static void TOF400C_Write_Word_Data(unsigned char reg, unsigned int dat);
static void TOF400C_Read_Word_Data(unsigned char reg, unsigned int *dat);
static unsigned char Check_TOF400C(void);

// TOF400C配置参数
unsigned char VL51L1X_CONFIGURATION[] =
{
    0x29, 0x02, 0x10, 0x00, 0x25, 0xbc, 0x51, 0x81, // 0`7
    0x09, 0x07, 0x93, 0x00, 0xff, 0xff, 0x75, 0xff, // 8`15
    0xfe, 0x0d, 0x00, 0x17, 0x01, 0x00, 0x00, 0x00, // 16`23
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 24`31
    0x30, 0x00, 0x17, 0x0a, 0x00, 0x00, 0x00, 0x00, // 32`39
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, // 40`47
    0x02, 0x00, 0x02, 0x08, 0x00, 0x08, 0x10, 0x01, // 48`55
    0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x02, // 56`63
    0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x0b, 0x00, // 64`71
    0x00, 0x02, 0xff, 0x21, 0x00, 0x00, 0x01, 0x00, // 72`79
    0x00, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x38, 0xff, // 80`87
    0x01, 0x00, 0x27, 0x00, 0x34, 0x00, 0x65, 0x07, // 88`95
    0x00, 0x87, 0x05, 0x01, 0x68, 0x00, 0xc0, 0x08, // 96`103
    0x38, 0x00, 0x00, 0x00, 0x00, 0x99, 0xc8, 0x00, // 104`111
    0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x07, // 112`119
    0x05, 0x06, 0x06, 0x03, 0x00, 0x02, 0xc7, 0xff, // 120`127
    0xdb, 0x02, 0x00, 0x00, 0x01, 0x01, 0x21,       // 128`134
};

/**
*
* @brief    TOF400C模块初始化
* @param    void
* @return   void
* @notes
* Example:  Init_TOF400C();
*
**/
void Init_TOF400C(void)
{
    unsigned char tmp = 0;
    // 初始化超时
    char time = TOF400C_TIMEOUT_COUNT;
    init_gpio( TOF400C_SCL_PIN , GPO_OD , Speed_50MHZ , 1);
    init_gpio( TOF400C_SDA_PIN , GPO_OD , Speed_50MHZ , 1);
    while(tmp == 0)
    {
        TOF400C_Write_Datas(VL53L1_I2C_SLAVE__DEVICE_ADDRESS, VL51L1X_CONFIGURATION, 135);
        switch(TOF400C_FREQ)
        {
        case 100:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0065);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0087);
            break;
        case 66:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x001D);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0027);
            break;
        case 50:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0051);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x006E);
            break;
        case 30:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x00D6);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x006E);
            break;
        case 20:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x01AE);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x01E8);
            break;
        case 10:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x02E1);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0388);
            break;
        case 5:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x03E1);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0496);
            break;
        case 2:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0591);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x05C1);
            break;
        default:
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_A_HI, 0x0065);
            TOF400C_Write_Word_Data(RANGE_CONFIG__TIMEOUT_MACROP_B_HI, 0x0087);
            break;
        }
        tmp = Check_TOF400C();
        time--;
        if(time < 0)
        {
            switch(output_device_flag)
            {
                case 0:printf("TOF400C Init Error!\r\n");break;
                case 1:Show_String_OLED(0,0,"TOF400C Init Error!",Show6x8);break;
                case 2:Show_String_IPS(0,0,"TOF400C Init Error!",BLACK,WHITE,Show8x16);break;
                case 3:Show_String_TFT180(0,0,"TOF400C Init Error!",BLACK,WHITE,Show8x16);break;
                default:printf("TOF400C Init Error!\r\n");break;
            }
            while(1);
        }
        TOF400C_DELAY_MS(100);
    }
    TOF400C_Write_Data(SYSTEM__INTERRUPT_CLEAR, 0x21);
}

/**
*
* @brief    获取TOF400C模块测量距离
* @param    void
* @return   unsigned int            测量出的距离,单位(mm)
* @notes
* Example:  Get_Distance_TOF400C();
*
**/
unsigned int Get_Distance_TOF400C(void)
{
    unsigned char tmp;
    unsigned int tof400c_distance;
    TOF400C_Write_Data(SYSTEM__INTERRUPT_CLEAR, 0x01);
    TOF400C_Read_Data(RESULT__RANGE_STATUS, &tmp);
    if(tmp == 137)
    {
        TOF400C_Read_Word_Data(VL53L1_RESULT__FINAL_CROSSTALK_CORRECTED_RANGE_MM_SD0, &tof400c_distance);
        if(tof400c_distance > 1400 || tof400c_distance < 0)
            tof400c_distance = 1400;
    }
    else
        tof400c_distance = 1400;

    return tof400c_distance;
}

/**
*
* @brief    获取中断极性
* @param    void
* @return   unsigned char               中断极性
* @notes    Init_TOF400C();函数中已调用,不可单独调用
* Example:  Get_IntPol_TOF400C();
*
**/
static unsigned char Get_IntPol_TOF400C(void)
{
    unsigned char temp;
    unsigned char int_pol;
    TOF400C_Read_Data(GPIO_HV_MUX__CTRL, &temp);
    temp = temp & 0x10;
    int_pol = !(temp >> 4);
    return int_pol;
}

/**
*
* @brief    查看数据是否准备就绪,准备就绪返回1反之返回0
* @param    void
* @return   unsigned char               准备就绪返回1反之返回0
* @notes    Init_TOF400C();函数中已调用,不可单独调用
* Example:  Check_TOF400C();
*
**/
static unsigned char Check_TOF400C(void)
{
    unsigned char temp;
    unsigned char int_pol;
    int_pol = Get_IntPol_TOF400C();
    TOF400C_Read_Data(GPIO__TIO_HV_STATUS, &temp);
    TOF400C_DELAY_MS(1);
    if ((temp & 1) == int_pol)
        return  1;
    else
        return  0;
}

/**
*
* @brief    TOF400C软件IIC开始
* @param    void
* @return   void
* @notes    内部调用
* Example:  TOF400C_Start();
*
**/
static void TOF400C_Start(void)
{
    TOF400C_SCL_LEVEL(1);
    TOF400C_SDA_LEVEL(1);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SDA_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SCL_LEVEL(0);
}

/**
*
* @brief    TOF400C软件IIC停止
* @param    void
* @return   void
* @notes    内部调用
* Example:  TOF400C_Stop();
*
**/
static void TOF400C_Stop(void)
{
    TOF400C_SCL_LEVEL(0);
    TOF400C_SDA_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SCL_LEVEL(1);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SDA_LEVEL(1);
    TOF400C_DELAY_MS(TOF400C_DELAY);
}

/**
*
* @brief    主机向从设备发送应答/非应答信号 1/0
* @param    ack             发送信号
* @return   void
* @notes    内部调用
* Example:  TOF400C_Sendack(1);
*
**/
static void TOF400C_Sendack(unsigned char ack)
{
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    if(ack)
        TOF400C_SDA_LEVEL(0);
    else
        TOF400C_SDA_LEVEL(1);
    TOF400C_SCL_LEVEL(1);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
}

/**
*
* @brief    主机等待从设备应答信号
* @param    void
* @return   int
* @notes    内部调用
* Example:  TOF400C_Waitack();
*
**/
static int TOF400C_Waitack(void)
{
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SCL_LEVEL(1);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    if(get_level_gpio(TOF400C_SDA_PIN))
    {
        TOF400C_SCL_LEVEL(1);
        return 0;
    }
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    return 1;
}

/**
*
* @brief    发送单个字节
* @param    ch                  字节
* @return   void
* @notes    内部调用
* Example:  TOF400C_Write_Char(0X66);
*
**/
static void TOF400C_Write_Char(unsigned char ch)
{
    unsigned char i = 8;
    while(i--)
    {
        if(ch & 0x80)
            TOF400C_SDA_LEVEL(1);
        else
            TOF400C_SDA_LEVEL(0);
        ch <<= 1;
        TOF400C_DELAY_MS(TOF400C_DELAY);
        TOF400C_SCL_LEVEL(1);
        TOF400C_DELAY_MS(TOF400C_DELAY);
        TOF400C_SCL_LEVEL(0);
    }
    TOF400C_Waitack();
}

/**
*
* @brief    接受一个字节数据
* @param    ack                 应答信号
* @return   unsigned char       接受字节
* @notes    内部调用
* Example:  TOF400C_Read_Char(0);
*
**/
static unsigned char TOF400C_Read_Char(unsigned char ack)
{
    unsigned char i;
    unsigned char c = 0;
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_SDA_LEVEL(1);
    for(i = 0; i < 8; i++)
    {
        TOF400C_DELAY_MS(TOF400C_DELAY);
        TOF400C_SCL_LEVEL(0);
        TOF400C_DELAY_MS(TOF400C_DELAY);
        TOF400C_SCL_LEVEL(1);
        TOF400C_DELAY_MS(TOF400C_DELAY);
        c <<= 1;
        if(get_level_gpio(TOF400C_SDA_PIN))
        {
            c += 1;
        }
    }
    TOF400C_SCL_LEVEL(0);
    TOF400C_DELAY_MS(TOF400C_DELAY);
    TOF400C_Sendack(ack);
    return c;
}

/**
*
* @brief    连续向指定寄存器写多个字节数据
* @param    reg                 寄存器
* @param    *dat                数据首地址
* @param    num                 写入数据的个数
* @return   void
* @notes    内部调用
* Example:  TOF400C_Write_Datas(0x0000,dat,1);
*
**/
static void TOF400C_Write_Datas(unsigned int reg, unsigned char *dat, unsigned long num)
{
    TOF400C_Start();
    TOF400C_Write_Char(VL53L1X_DEFAULT_DEVICE_ADDRESS);

    TOF400C_Write_Char((unsigned char)(reg >> 8));
    TOF400C_Write_Char(reg & 0x00ff);
    while(num--)
    {
        TOF400C_Write_Char(*dat++);
    }
    TOF400C_Stop();
}

/**
*
* @brief    连续从指定寄存器读多个字节数据
* @param    reg                 寄存器
* @param    *dat                数据首地址
* @param    num                 读取数据的个数
* @return   void
* @notes    内部调用
* Example:  TOF400C_Read_Datas(0x0000,dat,1);
*
**/
static void TOF400C_Read_Datas(unsigned int reg, unsigned char *dat, unsigned long num)
{
    unsigned char i = 0;
    TOF400C_Start();
    TOF400C_Write_Char(VL53L1X_DEFAULT_DEVICE_ADDRESS);

    TOF400C_Write_Char((unsigned char)(reg >> 8));
    TOF400C_Write_Char(reg & 0x00ff);

    TOF400C_Start();
    TOF400C_Write_Char(VL53L1X_DEFAULT_DEVICE_ADDRESS + 1);

    for(i = 0; i < num; i++)
    {
        if(i != (num - 1))
        {
            dat[i] = TOF400C_Read_Char(1);
        }
        else
        {
            dat[i] = TOF400C_Read_Char(0);
        }
    }
    TOF400C_Stop();
}

/**
*
* @brief    向指定寄存器写入1个字节数据
* @param    reg                 寄存器
* @param    dat                 1字节数据
* @return   void
* @notes    内部调用
* Example:  TOF400C_Write_Data(0x00,dat);
*
**/
static void TOF400C_Write_Data(unsigned char reg, unsigned char dat)
{
    TOF400C_Write_Datas(reg, &dat, 1);
}

/**
*
* @brief    从指定寄存器读取1个字节数据
* @param    reg                 寄存器
* @param    *dat                1字节数据地址
* @return   void
* @notes    内部调用
* Example:  TOF400C_Read_Data(0x00,dat);
*
**/
static void TOF400C_Read_Data(unsigned char  reg, unsigned char *dat)
{
    TOF400C_Read_Datas(reg, dat, 1);
}

/**
*
* @brief    向指定寄存器写入2个字节数据
* @param    reg                 寄存器
* @param    dat                 2字节数据
* @return   void
* @notes    内部调用
* Example:  TOF400C_Write_Word_Data(0x0000,dat);
*
**/
static void TOF400C_Write_Word_Data(unsigned char reg, unsigned int dat)
{
    unsigned char buf[2];
    buf[0] = dat >> 8;
    buf[1] = dat & 0x00FF;
    TOF400C_Write_Datas(reg, (unsigned char *)buf, 2);
}

/**
*
* @brief    从指定寄存器读取2个字节数据
* @param    reg                 寄存器
* @param    *dat                2字节数据地址
* @return   void
* @notes    内部调用
* Example:  TOF400C_Read_Word_Data(0x0000,dat);
*
**/
static void TOF400C_Read_Word_Data(unsigned char reg, unsigned int *dat)
{
    unsigned char buf[2] = {0, 0};
    TOF400C_Read_Datas(reg, buf, 2);
    *dat = (buf[0] << 8) + buf[1];
}
