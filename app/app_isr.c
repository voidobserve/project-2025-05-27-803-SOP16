/****************************************************************************
@FILENAME:	isr.c
@FUNCTION:	isr handler,stong isr function define
@AUTHOR:	flege.zhang
@DATE:		2020.04.18
*****************************************************************************/

#include "app_cfg.h"
#include "ble_adv.h"
#include "timer.h"
#include "timer_mgt.h"
#include "tccr.h"
#include "int.h"
#include "watchdog.h"
#include "uart.h"
#include "rcscan.h"
#include "key_process.h"

#include "user_config.h"
#include "user_key.h"

#ifndef SYSTEM_NOT_SLEEP
extern volatile uint8_t sys_sleep_enable;
extern volatile uint32_t sys_sleep_count;
#endif

ATTRIBUTE_ISR void tx_isr()
{
}

ATTRIBUTE_ISR void rx_isr()
{
}

ATTRIBUTE_ISR void gpio_isr()
{
}

ATTRIBUTE_ISR void timer0_isr()
{
    if (read_reg(TIMER0_INT_STATUS))
    {
        read_reg(TIMER0_EOI); // read clear

        // 更新扫描时间：
        if (touch_key_para.cur_scan_times < 255)  // 已经验证过这里是1ms进入一次
        {
            touch_key_para.cur_scan_times++; 
        }
    }
}

ATTRIBUTE_ISR void timer1_isr()
{
    if (read_reg(TIMER1_INT_STATUS))
    {
        read_reg(TIMER1_EOI); // read clear
        // ble_adv_start();
    }
}

ATTRIBUTE_ISR void timer2_isr()
{
    if (read_reg(TIMER2_INT_STATUS))
    {
        read_reg(TIMER2_EOI); // read clear
    }
}

ATTRIBUTE_ISR void core_timer_isr()
{
    read_reg(CORE_TIMER_CORET_CSR_REG); // read clear
    hal_rtc_core_timer_handler();
}

ATTRIBUTE_ISR void tccr_isr()
{
    /*
    __      ______
      |     |     |
      |_____|     |_________________________
      A     B     A       C
      A:irq,tccr timer reload,record current timer count as TCCR_CC1C_ADDR
      B:tccr record current timer count as TCCR_CC0C_ADDR
      C:irq, tccr timeout
    */
}

ATTRIBUTE_ISR void lvd_isr()
{
}

ATTRIBUTE_ISR void wdt_isr()
{
    if (read_reg(WATCHDOG_STATE_REG))
    {
        wdg_int_clear();
        wdg_feed_dog();
    }
}

extern uint8_t uart_getc(void);

// extern u8 uart_recv_buf[256];
// extern u32 uart_recv_key_val; // 存放接收到的按键键值

enum
{
    UART_RECV_STATUS_NONE = 0,    // 没有数据到来，无状态或是默认状态
    UART_RECV_STATUS_FORMAT_HEAD, // 表示接收到了格式头
    UART_RECV_STATUS_DATA_1,
    UART_RECV_STATUS_DATA_2,
    UART_RECV_STATUS_DATA_3,
    UART_RECV_STATUS_DATA_4,
};

ATTRIBUTE_ISR void uart_isr(void)
{
    u32 reg_val = 0;

#if 1
    /*RX interrupt */

    reg_val = read_reg(UART_INT_REG);

    if (UART_RX_INT_MASK & reg_val)
    {
        static u8 uart_recv_status = 0; // 状态机，表示当前接收状态
        static u8 uart_recv_cnt = 0;
        uint8_t data = uart_getc();

        sys_sleep_count = 0; // 串口收到信息，清空睡眠计时

        // 未收到数据，准备接收格式头
        if (UART_RECV_STATUS_NONE == uart_recv_status)
        {
            uart_recv_status = UART_RECV_STATUS_FORMAT_HEAD;
            uart_recv_buf[0] = data; // 存放格式头
            uart_recv_cnt++;

            if (FORMAT_HEAD_CIRCULAR_BUTTONS == data)
            {
            }
            else if (FORMAT_HEAD_INDEPENDENT_BUTTON == data)
            {
            }
            else
            {
                // 格式不正确，重新接收
                uart_recv_status = UART_RECV_STATUS_NONE;
                uart_recv_cnt = 0;
            }
        }
        else
        {
            uart_recv_buf[uart_recv_cnt] = data;
            uart_recv_cnt++;
            if (uart_recv_cnt >= 2)
            {
                uart_recv_cnt = 0;
                uart_recv_key_val = (u16)uart_recv_buf[0] << 8 |
                                    (u16)uart_recv_buf[1] << 0;
                uart_recv_status = UART_RECV_STATUS_NONE;
                flag_is_recv_uart = 1; // 表示收到了数据
            }
        }
    }

#endif

    /*TX interrupt*/
    // else if (UART_TX_INT_MASK & reg_val)
    // {
    // }
    write_reg(UART_INT_REG, UART_RX_INT_MASK | UART_TX_INT_MASK);
}
