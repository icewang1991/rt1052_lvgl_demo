#include "touch_drv.h"
#include "fsl_debug_console.h"
#include "lvgl.h"
#include "lv_demo_widgets.h"
#include "fsl_tsc.h"
#include "fsl_adc.h"
#include <string.h>

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define DEMO_ADC_BASE ADC2
#define DEMO_TSC_BASE TSC

#define EXAMPLE_TSC_IRQHANDLER TSC_DIG_IRQHandler

#define X_SCREEN_SIZE			480
#define Y_SCREEN_SIZE			272
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
volatile bool g_tscTouch = false;
uint16_t  x_start_ad=0;
uint16_t  x_end_ad=0;
uint16_t	y_start_ad=0;
uint16_t	y_end_ad=0;
uint32_t	x_scaler=0;
uint32_t	y_scaler=0;
void ADC_Configuration(void);
void NVIC_Configuration(void);
/*!
* @brief TSC IRQ handler.
*/
void EXAMPLE_TSC_IRQHANDLER(void)
{
    if ((TSC_GetInterruptStatusFlags(DEMO_TSC_BASE) & kTSC_DetectSignalFlag) == kTSC_DetectSignalFlag)
    {
        TSC_ClearInterruptStatusFlags(DEMO_TSC_BASE, kTSC_DetectSignalFlag);
        TSC_StartMeasure(DEMO_TSC_BASE);
    }
    else
    {
        if ((TSC_GetInterruptStatusFlags(DEMO_TSC_BASE) & kTSC_ValidSignalFlag) == kTSC_ValidSignalFlag)
        {
            TSC_ClearInterruptStatusFlags(DEMO_TSC_BASE, kTSC_ValidSignalFlag);
            g_tscTouch = true;
        }
        TSC_ClearInterruptStatusFlags(DEMO_TSC_BASE, kTSC_MeasureSignalFlag);
    }
}

/*!
* @brief Set configuration of ADC working with TSC.
*/
void ADC_Configuration(void)
{
    adc_config_t adcConfigStrcut;
    adc_channel_config_t adcChannelConfigStruct;

    /* Initialize the ADC module. */
    ADC_GetDefaultConfig(&adcConfigStrcut);
    ADC_Init(DEMO_ADC_BASE, &adcConfigStrcut);
#if !(defined(FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE) && FSL_FEATURE_ADC_SUPPORT_HARDWARE_TRIGGER_REMOVE)
    ADC_EnableHardwareTrigger(DEMO_ADC_BASE, true);
#endif

    /* Before TSC starts work, software driver configure ADC_HCx.
     * For four-wire resistive screen, x-coordinate measure triggers to ADC_HC3
     * and y-coordinate measure triggers to ADC_HC1. So we need configure ADC_HC1
     * and ADC_HC3 to make sure TSC could work normally.
     */
    adcChannelConfigStruct.channelNumber = 1U; /* Channel1 is ynlr port. */
    adcChannelConfigStruct.enableInterruptOnConversionCompleted = false;
    ADC_SetChannelConfig(DEMO_ADC_BASE, 3U, &adcChannelConfigStruct);
    adcChannelConfigStruct.channelNumber = 3U; /* Channel3 is xnur port. */
    ADC_SetChannelConfig(DEMO_ADC_BASE, 1U, &adcChannelConfigStruct);

    /* Do auto hardware calibration. */
    if (kStatus_Success == ADC_DoAutoCalibration(DEMO_ADC_BASE))
    {
        PRINTF("ADC_DoAntoCalibration() Done.\r\n");
    }
    else
    {
        PRINTF("ADC_DoAntoCalibration() Failed.\r\n");
    }
}

/*!
* @brief Set NVIC configuration.
*/
void NVIC_Configuration(void)
{
    NVIC_EnableIRQ(TSC_DIG_IRQn);
}

void touch_drv_init(void)
{
	tsc_config_t k_tscConfig;
	ADC_Configuration();
	NVIC_Configuration();

	TSC_GetDefaultConfig(&k_tscConfig);
	TSC_Init(DEMO_TSC_BASE, &k_tscConfig);
	/* Please enable interrupts and corresponding interrupt signals to insure proper interrupt operation.  */
	TSC_EnableInterruptSignals(DEMO_TSC_BASE,
														 kTSC_ValidSignalEnable | kTSC_MeasureSignalEnable | kTSC_DetectSignalEnable);
	TSC_EnableInterrupts(DEMO_TSC_BASE, kTSC_MeasureInterruptEnable | kTSC_DetectInterruptEnable);
	PRINTF("Please touch screen.\r\n");
	g_tscTouch = false;
}

bool touch_drv_is_pressed(void)
{
		TSC_StartSenseDetection(DEMO_TSC_BASE);
		while ((TSC_GetStatusFlags(DEMO_TSC_BASE) & kTSC_StateMachineFlag) != kTSC_IdleState)
		{
		}
		return g_tscTouch;
}

void touch_drv_get_xy(sTouchDrv* dev)
{
	uint32_t x_ad;
	uint32_t y_ad;
//	TSC_SoftwareReset(DEMO_TSC_BASE);
//	TSC_StartSenseDetection(DEMO_TSC_BASE);
	
	if (g_tscTouch)
  {
	g_tscTouch = false;
	x_ad = TSC_GetMeasureValue(DEMO_TSC_BASE, kTSC_XCoordinateValueSelection);
	y_ad = TSC_GetMeasureValue(DEMO_TSC_BASE, kTSC_YCoordinateValueSelection);
	TSC_SoftwareReset(DEMO_TSC_BASE);
	PRINTF("x = %d, y = %d\r\n", x_ad,y_ad);
	((x_ad <=x_start_ad)&&(x_ad>=x_end_ad))?(dev->x_pos=(x_start_ad-x_ad)/x_scaler):(dev->x_pos);
	((y_ad <=y_end_ad)&&(y_ad>=y_start_ad))?(dev->y_pos=(y_ad-y_start_ad)/y_scaler):(dev->y_pos);
	}
}

void calib_touch(void)
{
		uint8_t step=0;
		static lv_style_t style;
 
    lv_style_init(&style); // 样式初始化
    lv_style_reset(&style); // 重置样式
 
    lv_style_set_line_color(&style, lv_palette_main(LV_PALETTE_BLUE));
    lv_style_set_line_width(&style, 2); // 设置先宽
    lv_style_set_line_rounded(&style, true); // 设置线端到角度
		lv_point_t point1[] = {{0, 0}, {6, 0},{0, 0},{0, 6}};
		lv_point_t point2[] = {{473, 0}, {479, 0},{479, 0},{479, 6}};
		lv_point_t point3[] = {{0, 265}, {0, 271},{0, 271},{6, 271}};
	
		lv_obj_t * obj = lv_line_create(lv_scr_act());
		while(step<4){
		switch(step)
		{
			case 0:
					if (obj != NULL)
					{
							lv_obj_add_style(obj, &style, 0);
							lv_line_set_points(obj, point1, 4);
					}
					
					while(1){
						TSC_SoftwareReset(DEMO_TSC_BASE);  /* There is a misstake here by sw_reset but no wait complete */
						TSC_StartSenseDetection(DEMO_TSC_BASE);
						while ((TSC_GetStatusFlags(DEMO_TSC_BASE) & kTSC_StateMachineFlag) != kTSC_IdleState)
						{
							lv_task_handler();
						}
						if (g_tscTouch){
							 g_tscTouch = false;
							x_start_ad=TSC_GetMeasureValue(TSC, kTSC_XCoordinateValueSelection);
							y_start_ad=TSC_GetMeasureValue(TSC, kTSC_YCoordinateValueSelection);
							step = 1;
							break;
						}
					}
				break;
			case 1:
				if (obj != NULL)
					{
							lv_obj_add_style(obj, &style, 0);
							lv_line_set_points(obj, point2, 4);
					}
					while(1){
						TSC_SoftwareReset(DEMO_TSC_BASE);  /* There is a misstake here by sw_reset but no wait complete */
						TSC_StartSenseDetection(DEMO_TSC_BASE);
						while ((TSC_GetStatusFlags(DEMO_TSC_BASE) & kTSC_StateMachineFlag) != kTSC_IdleState)
						{
							lv_task_handler();
						}
						if (g_tscTouch){
							 g_tscTouch = false;
							x_end_ad=TSC_GetMeasureValue(TSC, kTSC_XCoordinateValueSelection);
							step = 2;
							break;
						}
					}
				break;
			case 2:
				if (obj != NULL)
					{
							lv_obj_add_style(obj, &style, 0);
							lv_line_set_points(obj, point3, 4);
					}
					while(1){
						TSC_SoftwareReset(DEMO_TSC_BASE);  /* There is a misstake here by sw_reset but no wait complete */
						TSC_StartSenseDetection(DEMO_TSC_BASE);
						while ((TSC_GetStatusFlags(DEMO_TSC_BASE) & kTSC_StateMachineFlag) != kTSC_IdleState)
						{
							lv_task_handler();
						}
						if (g_tscTouch){
							 g_tscTouch = false;
							y_end_ad=TSC_GetMeasureValue(TSC, kTSC_YCoordinateValueSelection);
							step = 3;
							break;
						}
					}
				break;
			case 3:
				x_scaler = (x_start_ad-x_end_ad)/X_SCREEN_SIZE;
				y_scaler = (y_end_ad-y_start_ad)/Y_SCREEN_SIZE;
				TSC_SoftwareReset(DEMO_TSC_BASE);
				step = 4;
				break;
		}
	}
}