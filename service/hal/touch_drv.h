#ifndef __TOUCH_DRV_H__
#define __TOUCH_DRV_H__
#include <stdint.h>
#include <stdbool.h>
typedef struct{
	uint16_t x_pos;
	uint16_t y_pos;
}sTouchDrv;
void touch_drv_init(void);

bool touch_drv_is_pressed(void);

void touch_drv_get_xy(sTouchDrv* dev);

void calib_touch(void);
#endif
