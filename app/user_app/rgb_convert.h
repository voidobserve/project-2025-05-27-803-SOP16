#ifndef __RGB_CONVERT_H
#define __RGB_CONVERT_H

#include "type.h"

enum
{
    CUR_SEL_RGB_MODE = 0,
    CUR_SEL_CW_MODE,
};

// 存放待发送的RGB值
extern volatile u8 r;
extern volatile u8 g;
extern volatile u8 b;

// 存放待发送的cw值
extern volatile u8 cw;

// 存放遥控器当前要调节的模式(对应遥控器上的色环)
// extern volatile u8 cur_sel_rgb_cw_mode;

extern const u8 table[][3];

extern str_retention_memory ret_mem_data;

void convert_key_val_to_rgb(u8 key_val);

#endif
