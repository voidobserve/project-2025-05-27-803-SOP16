#ifndef __USER_KEY_H
#define __USER_KEY_H

#include "user_config.h"
#include "key_driver.h"

/*
    触摸按键的扫描周期，单位：ms
    用在定时器中断，注意不能超过变量的最大值
*/
#define TOUCH_KEY_SCAN_CIRCLE_TIMES (20)
#define TOUCH_KEY_FILTER_TIMES (0)         // 触摸按键消抖次数
#define LONG_PRESS_TIME_THRESHOLD_MS (500) // 长按时间阈值（单位：ms）
#define HOLD_PRESS_TIME_THRESHOLD_MS (150) // 长按持续(不松手)的时间阈值(单位：ms)，每隔 xx 时间认为有一次长按持续事件

enum TOUCH_KEY_ID
{
    TOUCH_KEY_ID_NONE = 0x00,
    TOUCH_KEY_ID_1 = 0x01,
    TOUCH_KEY_ID_2 = 0x02,
    TOUCH_KEY_ID_3 = 0x03,
    TOUCH_KEY_ID_4 = 0x04,
    TOUCH_KEY_ID_5 = 0x05,
    TOUCH_KEY_ID_6 = 0x06,
    TOUCH_KEY_ID_7 = 0x07,
    TOUCH_KEY_ID_8 = 0x08,
    TOUCH_KEY_ID_9 = 0x09,
    TOUCH_KEY_ID_10 = 0x0A,
};

extern volatile struct key_driver_para touch_key_para;

extern void touch_key_handle(void);
#endif