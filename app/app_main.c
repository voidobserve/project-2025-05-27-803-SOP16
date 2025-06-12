/* ---------------------------------------------------------------
** File name:		main.c
** Brief:
** COPYRIGHT(C) ZXW 2020
** ---------------------------------------------------------------
** Module Information:
**
** Revision History ----------------------------------------------
** ---------------------------------------------------------------
**  Date        Version     Action                  Author
** ---------------------------------------------------------------
**  2019-5-17   1.0         Created
**
** ---------------------------------------------------------------
*/

/* Includes -----------------------------------------------------*/
#include "type.h"
#include "sys.h"
#include "int.h"
#include "lvd.h"
#include "tccr.h"
#include "timer.h"
#include "delay.h"
#include "timer_mgt.h"
#include "wakeup_timer.h"
#include "rf.h"
#include "uart.h"
#include "rcscan.h"
#include "key_process.h"
#include "stdlib.h"
// #include "gpadc.h"
#if (SYS_CRASH_WTD_RESET_ENABLE)
#include "watchdog.h"
#endif
#include "hal_rtc.h"
#include "ble_adv.h"

#include "user_config.h"

/* Macro --------------------------------------------------------*/

/* Global Variables ---------------------------------------------*/

/* Extern -------------------------------------------------------*/

/* Functions ----------------------------------------------------*/

/*
 1. Global variables stored from RAM start address.
 2. Local variables stored from stack top.
*/

volatile u8 flag_is_recv_uart = 0;

#ifndef SYSTEM_NOT_SLEEP
volatile uint8_t sys_sleep_enable = 0x0;
volatile uint32_t sys_sleep_count = 0x0;
#endif

extern str_retention_memory ret_mem_data;
extern str_key_info key_info;

uint32_t rtc_rcscan_t = 0;
uint32_t rtc_adv_start_t = 0;
uint32_t rtc_sys_sleep_t = 0;

#ifndef SYSTEM_NOT_SLEEP
void sys_reset_state_check()
{
	// read sys reset cause
	uint32_t reg_value = read_reg(WAKEUP_SOURE_STATE);
	uint8_t *p_retmem = (uint8_t *)&ret_mem_data;

	// reset by wakeup timer
	if (reg_value == WAKEUP_BY_WUT)
	{
		// get saved state by word from retention memory
		for (uint8_t i = 0; i < 16; i++)
		{
			p_retmem[i] = *((volatile uint32_t *)(RETENTION_MEMEORY_ADDR + (i << 2)));
		}

#if LOG_MAJOR
		print("RESET WT\n");
#endif
	}
	// first power on
	else if (reg_value == WAKEUP_BY_FPO)
	{
		memset(p_retmem, 0x0, RETENTION_MEMEORY_SIZE);
		ret_mem_data.current_group_index = 0x01; // default group index is 0x01,master using 0x00

#if LOG_MAJOR
		print("RESET 1ST\n");
#endif
	}

#if LOG_SECONDARY
	// log out reterntion memory by byte
	uint8_t *p_u8retmem = (uint8_t *)&ret_mem_data;
	print("RETMEM:");
	for (uint8_t i = 0; i < 16; i++)
	{
		print("%x ", p_u8retmem[i]);
	}
	print("\r\n");
#endif
}
#endif

extern str_ble_viot_para ble_viot_para;

void user_init(void)
{
	// init io
	hal_gpio_init(); // 打开GPIO时钟
	// hal_gpio_set_port(SINGLE_LED_GPIO_INDEX,GPIO_OUTPUT, GPIO_HIGH); // 不能设置为高电平，在不按遥控器时，灯会一直闪

	// init rtc
	hal_rtc_init();

	// config unsleep
	write_reg(TOP_POWER_CTRL_REG, 0x00); // 需手动清除唤醒标志位，因为硬件不会自动清除
	// off wakeup timer
	wakeup_timer_disable();
	wt_int_clear();

#if (SYS_CRASH_WTD_RESET_ENABLE)
	// init watch dog
	wdg_int_clear();
	wdg_feed_dog();
	watchdog_init(WATCH_DOG_RESET_TIME);
#endif

#if (LOG_ENABLE)
	uart_init(UART_BAUDRATE_115200);
	uart_set_port(UART_PORT_TX1RX1);
#endif

#ifndef SYSTEM_NOT_SLEEP
	sys_reset_state_check();
#endif

	// lvd_init();
	// lvd_threshold_set(LVD_THR_3);
	{ // 关闭LVD模块
		uint32_t reg_val = 0;
		reg_val = read_reg(MODULE_MODE_REG);
		// reg_val |= LVD_EN;
		reg_val &= ~(0x01 << 5);
		write_reg(MODULE_MODE_REG, reg_val);
	}

	// key_process_init();
	// memset(&key_info, 0x0, sizeof(str_key_info)); // 初始化存放按键信息的变量
	ble_packet_init(); //

	// peripherals enable
#if ((PRODUCT_SELECTION & RCSCAN) == RCSCAN)
	// rcscan_init();
	// memset(&key_info, 0x0, sizeof(str_key_info));
#endif

	// init rf
	rf_init();
#if LOG_MAJOR
	print("system up!\r\n");
#endif

	// MY_DEBUG:
	// gpio_set_bit_direction(BIT1, GPIO_OUTPUT);
	// sys_set_port_mux(PAD_GPIO_01, PAD_MUX_FUNCTION_1); // TXD

	gpio_set_mode(BIT8, GPIO_PULL_UP); // 上拉
	gpio_set_bit_direction(BIT8, GPIO_INPUT);
	sys_set_port_mux(PAD_GPIO_08, PAD_MUX_FUNCTION_3); // 复用为 UART RXD
	uart_init(UART_BAUDRATE_115200);
	int_enable_irq(INT_UART_EN_MASK); // 使能串口中断

	timer0_init(TIMER_CLOCK_24M, 24000); // 初始化，1ms产生一次中断

	// // 测试引脚配置：
	// gpio_set_mode(BIT0, GPIO_PULL_UP); // 上拉
	// gpio_set_bit_direction(BIT0, GPIO_INPUT);

	// 上电后读取当前 cur_sel_rgb_cw_mode 的状态
	cur_sel_rgb_cw_mode = *((volatile uint32_t *)(RETENTION_MEMEORY_ADDR));
}

int main(void)
{
	// init sys
	sys_init();
	sys_set_clock(CPU_CLOCK_24M);
	user_init();

	// MY_DEBUG:
	// {
	// 	u32 reg_value = 0;
	// 	reg_value = read_reg(WAKEUP_SOURE_STATE); // 查询唤醒方式
	// 	if (reg_value == WAKEUP_BY_WUT)			  // 判断是否为IO唤醒
	// 	{
	// 		my_printf("RESET WT\n");
	// 		// my_printf("RESET WT\n");
	// 	}
	// 	else if (reg_value == WAKEUP_BY_FPO) // 判断是否为第一次上电启动
	// 	{
	// 		my_printf("RESET 1ST\n");
	// 		// my_printf("RESET 1ST\n");
	// 	}

	// 	my_printf("sys reset\n");
	// }

	while (1)
	{
		// my_printf("main\n");
		// u8 ret =  uart_get_rx_int_status();
		// my_printf("ret = %d\n", ret);
		// gpio_set_bit(BIT8, 0);
		// delay_ms(500);
		// gpio_set_bit(BIT8, 1);
		// delay_ms(500);

		// if (1) // 如果收到了按键信息
		// if (0 == gpio_get_bits(BIT0))
		// {
		// 	my_printf("key press\n");
		// 	my_printf("send msg\n");

		// 	// key_info.curr_key = 0xA5; // 存放当前的键值

		// 	// 短按调用的函数体：
		// 	ble_viot_para.cmd = 0xFF; // 表示直接发送按键键值
		// 	// ble_viot_para.para[0] = para->para_a;
		// 	// ble_viot_para.para[0] = 键值;
		// 	ble_viot_para.para[0] = 0xA5; // 键值固定为0xA5

		// 	ble_adv_start();
		// 	ble_packet_send(); // 发送数据包
		// }

		// if (flag_is_recv_uart)
		// {
		// 	my_printf("%x\n", uart_recv_key_val);
		// 	flag_is_recv_uart = 0;
		// }

		// if (uart_recv_key_val)
		// {
		// 	// my_printf("%d\n", uart_recv_key_val);
		// 	// // my_printf("%d%d\n", uart_recv_key_val >> 8, uart_recv_key_val & 0xFF);
		// 	// uart_recv_key_val = 0;

		// 	// 短按调用的函数体：
		// 	ble_viot_para.cmd = 0xFF; // 表示直接发送按键键值
		// 	// ble_viot_para.para[0] = para->para_a;
		// 	// ble_viot_para.para[0] = 键值;
		// 	ble_viot_para.para[0] = (uart_recv_key_val >> 8) & 0xFF;
		// 	ble_viot_para.para[1] = uart_recv_key_val & 0xFF;
		// 	uart_recv_key_val = 0; // 清除接收到的键值

		// 	ble_adv_start();
		// 	ble_packet_send(); // 发送数据包

		// 	sys_sleep_count = 0; // 有按键按下，清空睡眠计时
		// }

		key_driver_scan((void *)&touch_key_para);
		touch_key_handle();

		if (hal_clock_time_exceed_rtc(&rtc_adv_start_t, 10000)) // 10 ms扫描一次广播发送
		{
			ble_adv_start();
		}

		// generate rand seed!!don't delete it!!
		ret_mem_data.rand_seed += rand();
#if (SYS_CRASH_WTD_RESET_ENABLE)
		wdg_feed_dog();
#endif

#ifndef SYSTEM_NOT_SLEEP
		// 5ms累计一次休眠倒计时
		if (hal_clock_time_exceed_rtc(&rtc_sys_sleep_t, 5000))
		{
			sys_sleep_count += TIMER0_IRQ_INTERVAL;

			if (sys_sleep_count >= SYS_SLEEP_TIME_MS)
			{
				// MY_DEBUG:
				sys_sleep_enable = 1;

				sys_sleep_count = 0;
			}
		}

		/*sleep when conditions are met*/
		if (sys_sleep_enable)
		{
			sys_sleep_count = 0;
			sys_sleep_enable = 0;

			// my_printf("enter sleep\r\n");
#if LOG_MAJOR
			print("enter sleep\r\n");
#endif
			// config not wakeup io as output low,for power saving
			// hal_gpio_cfg_before_sleep(UNUSED_IO);

			// 休眠前，写入 current_rgb_mode 的状态：
			// RETENTION_MEMEORY_ADDR 地址是不连续，偏移地址为0x4N，且必须以字节方式操作
			uint8_t *p_retmem = (uint8_t *)&ret_mem_data;
			for (uint8_t i = 0; i < 16; i++)
			{
				*((volatile uint32_t *)(RETENTION_MEMEORY_ADDR + (i << 2))) = p_retmem[i];
			}

			// *((volatile uint32_t *)(RETENTION_MEMEORY_ADDR)) = cur_sel_rgb_cw_mode;

#if (SYS_CRASH_WTD_RESET_ENABLE) // 关闭看门狗： //  disable wtd
								
			wdg_int_clear();
			wdg_feed_dog();
			watchdog_disable();
#endif

			hal_gpio_cfg_before_sleep(~(BIT8)); // 配置其它不作为唤醒的IO为debug模式，必须
			// 配置唤醒引脚
			sys_soft_reset(UART_SOFT_RESET_MASK);
			sys_set_module_clock(CLK_UART_EN_MASK, OFF);	   // 关闭串口时钟
			int_disable_irq(INT_UART_EN_MASK);				   // 不使能串口中断
			sys_set_port_mux(PAD_GPIO_08, PAD_MUX_FUNCTION_0); // 复用为IO
			gpio_set_mode(BIT8, GPIO_PULL_UP);				   // 上拉
			gpio_set_bit_direction(BIT8, GPIO_INPUT);
			wakeup_timer_init(BIT8, 0x1, 0x0, 0x1); // 上拉，低电平唤醒 (保守要10ms才能唤醒)

			// 进入休眠，唤醒是重新跑main函数的
			sys_sleep_down();

			// 必须要延时10ms
			// 当 MCU 从休眠态被唤醒时，需要等待一个振荡稳定时间（ Reset Time） ,这个时间标称值为 5ms。
			// delay_ms(10);
			delay_ms(5);
			user_init();

			// my_printf("unsleep\r\n");
#if LOG_ERROR
			print("unsleep\n");
#endif
		} // if (sys_sleep_enable)
#endif
	} // while (1)
}
