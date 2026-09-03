#include "bsp_joystick.h"

#include "app_config.h"
#include "ch32v30x.h"

static int8_t joystick_x_dir;
static int8_t joystick_y_dir;

#if USE_JOYSTICK
static uint16_t BSP_Joystick_ReadChannel(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1,
                             channel,
                             1U,
                             ADC_SampleTime_41Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
    {
    }
    return ADC_GetConversionValue(ADC1);
}

static int8_t BSP_Joystick_ToDirection(uint16_t value)
{
    if(value < JOYSTICK_LOW_THRESHOLD)
    {
        return -1;
    }
    if(value > JOYSTICK_HIGH_THRESHOLD)
    {
        return 1;
    }
    return 0;
}
#endif

void BSP_Joystick_Init(void)
{
    joystick_x_dir = 0;
    joystick_y_dir = 0;

#if USE_JOYSTICK
    GPIO_InitTypeDef gpio;
    ADC_InitTypeDef adc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |
                           RCC_APB2Periph_ADC1,
                           ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div8);

    gpio.GPIO_Pin = JOYSTICK_X_PIN | JOYSTICK_Y_PIN;
    gpio.GPIO_Mode = GPIO_Mode_AIN;
    gpio.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOA, &gpio);

    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = DISABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1U;
    adc.ADC_OutputBuffer = ADC_OutputBuffer_Disable;
    adc.ADC_Pga = ADC_Pga_1;
    ADC_Init(ADC1, &adc);

    ADC_Cmd(ADC1, ENABLE);
    ADC_BufferCmd(ADC1, DISABLE);
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1))
    {
    }
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1))
    {
    }
    ADC_BufferCmd(ADC1, ENABLE);
#endif
}

void BSP_Joystick_Update(void)
{
#if USE_JOYSTICK
    joystick_x_dir =
        BSP_Joystick_ToDirection(BSP_Joystick_ReadChannel(
            JOYSTICK_X_CHANNEL));
    joystick_y_dir =
        BSP_Joystick_ToDirection(BSP_Joystick_ReadChannel(
            JOYSTICK_Y_CHANNEL));
#else
    joystick_x_dir = 0;
    joystick_y_dir = 0;
#endif
}

int8_t BSP_Joystick_GetXDir(void)
{
    return joystick_x_dir;
}

int8_t BSP_Joystick_GetYDir(void)
{
    return joystick_y_dir;
}
