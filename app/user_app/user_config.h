#ifndef __USER_CONFIG_H
#define __USER_CONFIG_H

// #include <stdio.h>

#include "type.h"
#include "config.h"

#include "key_driver.h"
#include "user_key.h"
#include "rgb_convert.h"



// #include <stdio.h>

#define ARRAY_SIZE(array) (sizeof(array)/sizeof(array[0]))

#define FORMAT_HEAD_INDEPENDENT_BUTTON (0x52) // 独立触摸按键的格式头
#define FORMAT_HEAD_CIRCULAR_BUTTONS (0x51)

extern volatile u8 flag_is_recv_uart; // 

#endif

