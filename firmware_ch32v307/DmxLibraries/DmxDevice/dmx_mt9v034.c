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
 * @file       dmx_mt9v034.c
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

#include "dmx_mt9v034.h"
#include "dmx_gpio.h"
#include "dmx_sccb.h"

#include "debug.h"
#include "dmx_oled.h"
#include "dmx_ips.h"
#include "dmx_tft180.h"

void DVP_IRQHandler (void) __attribute__((interrupt("WCH-Interrupt-fast")));

// 原始图像数据
unsigned char MT9V034_Image_Data[MT9V034_IMAGEH][MT9V034_IMAGEW];

// 写入到摄像头的数据,在dmx_mt9v034.h中已进行宏定义,此处不可修改
static int Config_MT9V034[12]=
{
    MT9V034_IMAGEW,                 // 列,摄像头图像宽
    MT9V034_IMAGEH,                 // 行,摄像头图像高
    CAMERA_FPS,                     // 图像帧率设置
    CAMERA_EXPOSURE_TIME,           // 曝光时间越长,图像越亮;时间过长,帧率会下降
    CAMERA_AUTO_EXPOSURE,           // 自动曝光开关0:关闭自动曝光,1:开启自动曝光
    CAMERA_AUTO_EXPOSURE_BRIGHTNESS,// 自动曝光模式下亮度调节范围(1~64)
    CAMERA_MAX_EXPOSURE_TIME,       // 最大曝光时间,修改这里可以修改比较暗时的图像整体亮度
    CAMERA_MIN_EXPOSURE_TIME,       // 最小曝光时间,修改这里可以修改遇到强光时的图像整体亮度
    CAMERA_CONTRAST,                // 对比度高低设置0:低对比度,1:高对比度
    CAMERA_ANALOG_GAIN,             // 图像模拟增益设置,16~64,图像亮暗程度
    CAMERA_LEVEL_OFFSET,            // 图像水平方向偏移量,左负右正
    CAMERA_VERTICAL_OFFSET,         // 图像垂直方向偏移量,下负上正
};

/**
*
* @brief    MT9V034摄像头初始化
* @param
* @return   void
* @notes
* Example:  Init_MT9V034();
*
**/
void Init_MT9V034(void)
{
    NVIC_InitTypeDef NVIC_InitStructure={0};
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DVP, ENABLE);

    // 初始化SCCB
    init_gpio( MT9V034_SCL_PIN , GPO_OD , Speed_50MHZ , 1);
    init_gpio( MT9V034_SDA_PIN , GPO_OD , Speed_50MHZ , 1);

    // 读取摄像头ID
    unsigned short read;
    read = Set_Config_MT9V034(Config_MT9V034,0);
    if( read == 0XFFFF)
    {
        switch(output_device_flag)
        {
            case 0:printf("Camear Init Error!\r\n");break;
            case 1:Show_String_OLED(0,0,"Camear Init Error!",Show6x8);break;
            case 2:Show_String_IPS(0,0,"Camear Init Error!",BLACK,WHITE,Show8x16);break;
            case 3:Show_String_TFT180(0,0,"Camear Init Error!",BLACK,WHITE,Show8x16);break;
            default:printf("Camear Init Error!\r\n");break;
        }
        while (1);          // SCCB通讯失败,停止运行
    }
    else if(read != 0X1324)
    {
        switch(output_device_flag)
        {
            case 0:printf("Camear ID Error!\r\n");break;
            case 1:Show_String_OLED(0,0,"Camear ID Error!",Show6x8);break;
            case 2:Show_String_IPS(0,0,"Camear ID Error!",BLACK,WHITE,Show8x16);break;
            case 3:Show_String_TFT180(0,0,"Camear ID Error!",BLACK,WHITE,Show8x16);break;
            default:printf("Camear ID Error!\r\n");break;
        }
        while (1);          // 芯片ID不正确,没有正确读取到数据,检查摄像头接线
    }
    // 写命令到摄像头
    Set_Config_MT9V034(Config_MT9V034,1);

    // 初始化IO
    init_gpio( A6  , GPI_PU , Speed_50MHZ , 0); // CLK
    init_gpio( A5  , GPI_PU , Speed_50MHZ , 0); // VSY
    // HREF->A4
    init_gpio( A9  , GPI_PU , Speed_50MHZ , 0); // D0
    init_gpio( A10 , GPI_PU , Speed_50MHZ , 0); // D1
    init_gpio( C8  , GPI_PU , Speed_50MHZ , 0); // D2
    init_gpio( C9  , GPI_PU , Speed_50MHZ , 0); // D3
    init_gpio( C11 , GPI_PU , Speed_50MHZ , 0); // D4
    init_gpio( B6  , GPI_PU , Speed_50MHZ , 0); // D5
    init_gpio( B8  , GPI_PU , Speed_50MHZ , 0); // D6
    init_gpio( B9  , GPI_PU , Speed_50MHZ , 0); // D7

    DVP->CR0 &= ~RB_DVP_MSK_DAT_MOD;

    DVP->CR0 |= RB_DVP_D8_MOD | RB_DVP_V_POLAR;
    DVP->CR1 &= ~((RB_DVP_ALL_CLR)| RB_DVP_RCV_CLR);
    DVP->ROW_NUM = MT9V034_IMAGEH;
    DVP->COL_NUM = MT9V034_IMAGEW * MT9V034_IMAGEH;

    DVP->DMA_BUF0 = (unsigned int)MT9V034_Image_Data[0];
//    DVP->DMA_BUF1 = (unsigned int)MT9V034_Image_Data[0];

    DVP->CR1 &= ~RB_DVP_FCRC;
    DVP->CR1 |= DVP_RATE_100P;

    DVP->IER |= RB_DVP_IE_STP_FRM;
    DVP->IER |= RB_DVP_IE_FIFO_OV;
    DVP->IER |= RB_DVP_IE_FRM_DONE;
    DVP->IER |= RB_DVP_IE_ROW_DONE;
    DVP->IER |= RB_DVP_IE_STR_FRM;

    NVIC_InitStructure.NVIC_IRQChannel = DVP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DVP->CR1 |= RB_DVP_DMA_EN;
    DVP->CR0 |= RB_DVP_ENABLE;
}

/**
*
* @brief    MT9V034摄像头单独设置曝光时间
* @param    time                    曝光时间
* @param    gain                    图像增益
* @return   void
* @notes    使用前摄像头需要初始化
* Example:  Set_Exposure_Time_MT9V034(800,32);
*
**/
void Set_Exposure_Time_MT9V034(unsigned int time,unsigned int gain)
{
    Set_Exposure_Time(time,gain);
}

// DVP中断
void DVP_IRQHandler(void)
{
    if (DVP->IFR & RB_DVP_IF_ROW_DONE)
    {
        DVP->IFR &= ~RB_DVP_IF_ROW_DONE;
    }

    if (DVP->IFR & RB_DVP_IF_FRM_DONE)
    {
        DVP->IFR &= ~RB_DVP_IF_FRM_DONE;
    }

    if (DVP->IFR & RB_DVP_IF_STR_FRM)
    {
        DVP->IFR &= ~RB_DVP_IF_STR_FRM;
    }

    if (DVP->IFR & RB_DVP_IF_STP_FRM)
    {
        DVP->IFR &= ~RB_DVP_IF_STP_FRM;
    }

    if (DVP->IFR & RB_DVP_IF_FIFO_OV)
    {
        DVP->IFR &= ~RB_DVP_IF_FIFO_OV;
    }
}
