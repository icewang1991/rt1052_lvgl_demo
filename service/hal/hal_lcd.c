
#include "fsl_debug_console.h"
#include "fsl_elcdif.h"
#include "fsl_lpi2c.h"
#include "fsl_ft5406_rt.h"
#include "hal_lcd.h"
#include "fsl_common.h"

#define APP_ELCDIF LCDIF
#define APP_IMG_HEIGHT 272
#define APP_IMG_WIDTH 480
#define APP_HSW 41
#define APP_HFP 4
#define APP_HBP 8
#define APP_VSW 10
#define APP_VFP 4
#define APP_VBP 2
#define APP_POL_FLAGS \
    (kELCDIF_DataEnableActiveHigh | kELCDIF_VsyncActiveLow | kELCDIF_HsyncActiveLow | kELCDIF_DriveDataOnRisingClkEdge)
#define ELCDIF_PIXEL_FORMAT kELCDIF_PixelFormatRGB565
#define APP_LCDIF_DATA_BUS kELCDIF_DataBus16Bit
#define GUI_BUFFERS 2
#define LCD_WIDTH 480
#define LCD_HEIGHT 272
#define LCD_BITS_PER_PIXEL 16
#define LCD_BYTES_PER_PIXEL (LCD_BITS_PER_PIXEL / 8)
#define FRAME_BUFFER_ALIGN 64

/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)
#define BOARD_TOUCH_I2C_CLOCK_FREQ ((CLOCK_GetFreq(kCLOCK_Usb1PllClk) / 8) / (LPI2C_CLOCK_SOURCE_DIVIDER + 1U))
#define BOARD_TOUCH_I2C_BAUDRATE 100000U

/* Macros for the touch touch controller. */
#define BOARD_TOUCH_I2C LPI2C1

#define VRAM_SIZE (LCD_HEIGHT * LCD_WIDTH)
#define VRAM_ADDR ((uint32_t)s_vram_buffer)

AT_NONCACHEABLE_SECTION_ALIGN(uint16_t s_vram_buffer[LCD_HEIGHT][LCD_WIDTH], FRAME_BUFFER_ALIGN);
static volatile bool s_frameDone = false;
/*******************************************************************************
 * Code
 ******************************************************************************/
void BOARD_EnableLcdInterrupt(void)
{
    EnableIRQ(LCDIF_IRQn);
}

void APP_LCDIF_IRQHandler(void)
{
    uint32_t intStatus;

    intStatus = ELCDIF_GetInterruptStatus(APP_ELCDIF);

    ELCDIF_ClearInterruptStatus(APP_ELCDIF, intStatus);

    if (intStatus & kELCDIF_CurFrameDone)
    {
         s_frameDone = true;
    }
/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
  exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
    __DSB();
#endif
}

void LCDIF_IRQHandler(void)
{
    APP_LCDIF_IRQHandler();
}
void APP_ELCDIF_Init(void)
{
    const elcdif_rgb_mode_config_t config = {
        .panelWidth = APP_IMG_WIDTH,
        .panelHeight = APP_IMG_HEIGHT,
        .hsw = APP_HSW,
        .hfp = APP_HFP,
        .hbp = APP_HBP,
        .vsw = APP_VSW,
        .vfp = APP_VFP,
        .vbp = APP_VBP,
        .polarityFlags = APP_POL_FLAGS,
        .bufferAddr = VRAM_ADDR,
        .pixelFormat = ELCDIF_PIXEL_FORMAT,
        .dataBus = APP_LCDIF_DATA_BUS,
    };
    ELCDIF_RgbModeInit(APP_ELCDIF, &config);

#if (LCD_BITS_PER_PIXEL == 8)
    /* Load the LUT data. */
    ELCDIF_UpdateLut(APP_ELCDIF, kELCDIF_Lut0, 0, lutData, ELCDIF_LUT_ENTRY_NUM);
    ELCDIF_EnableLut(APP_ELCDIF, true);
#endif
		BOARD_EnableLcdInterrupt();
		ELCDIF_EnableInterrupts(APP_ELCDIF, kELCDIF_CurFrameDoneInterruptEnable);
    NVIC_EnableIRQ(LCDIF_IRQn);
    ELCDIF_RgbModeStart(APP_ELCDIF);
}
/*******************************************************************************
 * Implementation of communication with the touch controller
 ******************************************************************************/

/* Touch driver handle. */
//static ft5406_rt_handle_t touchHandle;

//static void BOARD_Touch_Init(void)
//{
//    lpi2c_master_config_t masterConfig = {0};
//    /*
//    * masterConfig.debugEnable = false;
//    * masterConfig.ignoreAck = false;
//    * masterConfig.pinConfig = kLPI2C_2PinOpenDrain;
//    * masterConfig.baudRate_Hz = 100000U;
//    * masterConfig.busIdleTimeout_ns = 0;
//    * masterConfig.pinLowTimeout_ns = 0;
//    * masterConfig.sdaGlitchFilterWidth_ns = 0;
//    * masterConfig.sclGlitchFilterWidth_ns = 0;
//    */
//    LPI2C_MasterGetDefaultConfig(&masterConfig);

//    /* Change the default baudrate configuration */
//    masterConfig.baudRate_Hz = BOARD_TOUCH_I2C_BAUDRATE;

//    /* Initialize the LPI2C master peripheral */
//    LPI2C_MasterInit(BOARD_TOUCH_I2C, &masterConfig, BOARD_TOUCH_I2C_CLOCK_FREQ);

//    /* Initialize the touch handle. */
//    FT5406_RT_Init(&touchHandle, BOARD_TOUCH_I2C);
//}

void BOARD_Touch_Deinit(void)
{
    LPI2C_MasterDeinit(BOARD_TOUCH_I2C);
}

int BOARD_Touch_Poll(void)
{
//    touch_event_t touch_event;
//    int touch_x;
//    int touch_y;
//    GUI_PID_STATE pid_state;

//    if (kStatus_Success != FT5406_RT_GetSingleTouch(&touchHandle, &touch_event, &touch_x, &touch_y))
//    {
//        return 0;
//    }
//    else if (touch_event != kTouch_Reserved)
//    {
//        pid_state.x = touch_y;
//        pid_state.y = touch_x;
//        pid_state.Pressed = ((touch_event == kTouch_Down) || (touch_event == kTouch_Contact));
//        pid_state.Layer = 0;
//        GUI_TOUCH_StoreStateEx(&pid_state);
//        return 1;
//    }
    return 0;
}
void APP_FillFrameBuffer(uint32_t x,uint32_t y,uint32_t x2,uint32_t y2,uint16_t* color)
{
	uint32_t i=0,j=0;
	uint32_t width;
	uint32_t hight;
	
	width = x2-x+1;
	hight	=	y2-y+1;
	
	for(i=0;i<hight;i++){
		for(j=0;j<width;j++){
			s_vram_buffer[y+i][x+j] = color[i*width+j];
		}
	}
	//ELCDIF_SetNextBufferAddr(APP_ELCDIF,VRAM_ADDR);
	s_frameDone = false;
	/* Wait for previous frame complete. */
	while (!s_frameDone)
	{
	}
}
