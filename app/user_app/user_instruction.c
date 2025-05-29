#include "user_instruction.h"

// 指令扫描
void instruction_scan(void)
{

}


// 指令处理
void instruction_handle(void)
{
    // 在发送2.4G信号前，先填充好数据包


    if (1) // 如果有对应的指令
    {
        ble_adv_start();
        ble_packet_send(); // 发送数据包
    }
}