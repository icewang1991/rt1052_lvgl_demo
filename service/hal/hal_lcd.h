#ifndef __HAL_LCD_H__
#define __HAL_LCD_H__
#include <stdint.h>

/* Display. */
#define LCD_DISP_GPIO GPIO1
#define LCD_DISP_GPIO_PIN 8
/* Back light. */
#define LCD_BL_GPIO GPIO1
#define LCD_BL_GPIO_PIN 10
/* Select USB1 PLL (480 MHz) as master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_SELECT (0U)
/* Clock divider for master lpi2c clock source */
#define LPI2C_CLOCK_SOURCE_DIVIDER (5U)

void APP_ELCDIF_Init(void);
static void BOARD_Touch_Init(void);
void BOARD_Touch_Deinit(void);
int BOARD_Touch_Poll(void);
void APP_FillFrameBuffer(uint32_t x,uint32_t y,uint32_t x2,uint32_t y2,uint16_t* color);
#endif
