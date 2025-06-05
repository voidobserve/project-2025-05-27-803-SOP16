
#include "user_key.h"

#include "ble_adv.h"         // ble_adv_start()
#include "ble_packet_send.h" // ble_packet_send()

extern volatile u16 uart_recv_key_val; // 存放接收到的按键键值

static volatile u8 cur_recv_data = 0;

// 定义触摸按键的按键事件
enum TOUCH_KEY_EVENT
{
    TOUCH_KEY_EVENT_NONE = 0,

    TOUCH_KEY_EVENT_ID_1_CLICK,
    TOUCH_KEY_EVENT_ID_1_LONG,
    TOUCH_KEY_EVENT_ID_1_HOLD,
    TOUCH_KEY_EVENT_ID_1_LOOSE,

    TOUCH_KEY_EVENT_ID_2_CLICK,
    TOUCH_KEY_EVENT_ID_2_LONG,
    TOUCH_KEY_EVENT_ID_2_HOLD,
    TOUCH_KEY_EVENT_ID_2_LOOSE,

    TOUCH_KEY_EVENT_ID_3_CLICK,
    TOUCH_KEY_EVENT_ID_3_LONG,
    TOUCH_KEY_EVENT_ID_3_HOLD,
    TOUCH_KEY_EVENT_ID_3_LOOSE,

    TOUCH_KEY_EVENT_ID_4_CLICK,
    TOUCH_KEY_EVENT_ID_4_LONG,
    TOUCH_KEY_EVENT_ID_4_HOLD,
    TOUCH_KEY_EVENT_ID_4_LOOSE,

    TOUCH_KEY_EVENT_ID_5_CLICK,
    TOUCH_KEY_EVENT_ID_5_LONG,
    TOUCH_KEY_EVENT_ID_5_HOLD,
    TOUCH_KEY_EVENT_ID_5_LOOSE,

    TOUCH_KEY_EVENT_ID_6_CLICK,
    TOUCH_KEY_EVENT_ID_6_LONG,
    TOUCH_KEY_EVENT_ID_6_HOLD,
    TOUCH_KEY_EVENT_ID_6_LOOSE,

    TOUCH_KEY_EVENT_ID_7_CLICK,
    TOUCH_KEY_EVENT_ID_7_LONG,
    TOUCH_KEY_EVENT_ID_7_HOLD,
    TOUCH_KEY_EVENT_ID_7_LOOSE,

    TOUCH_KEY_EVENT_ID_8_CLICK,
    TOUCH_KEY_EVENT_ID_8_LONG,
    TOUCH_KEY_EVENT_ID_8_HOLD,
    TOUCH_KEY_EVENT_ID_8_LOOSE,

    TOUCH_KEY_EVENT_ID_9_CLICK,
    TOUCH_KEY_EVENT_ID_9_LONG,
    TOUCH_KEY_EVENT_ID_9_HOLD,
    TOUCH_KEY_EVENT_ID_9_LOOSE,

    TOUCH_KEY_EVENT_ID_10_CLICK,
    TOUCH_KEY_EVENT_ID_10_LONG,
    TOUCH_KEY_EVENT_ID_10_HOLD,
    TOUCH_KEY_EVENT_ID_10_LOOSE,
};

#define TOUCH_KEY_EFFECT_EVENT_NUMS (4) // 单个触摸按键的有效按键事件个数
// 将按键id和按键事件绑定起来，在 xx 函数中，通过查表的方式得到按键事件
static const u8 touch_key_event_table[][TOUCH_KEY_EFFECT_EVENT_NUMS + 1] = {
    // [0]--按键对应的id号，用于查表，[1]、[2]、[3]...--用于与 key_driver.h 中定义的按键事件KEY_EVENT绑定关系(一定要一一对应)
    {TOUCH_KEY_ID_1, TOUCH_KEY_EVENT_ID_1_CLICK, TOUCH_KEY_EVENT_ID_1_LONG, TOUCH_KEY_EVENT_ID_1_HOLD, TOUCH_KEY_EVENT_ID_1_LOOSE},      //
    {TOUCH_KEY_ID_2, TOUCH_KEY_EVENT_ID_2_CLICK, TOUCH_KEY_EVENT_ID_2_LONG, TOUCH_KEY_EVENT_ID_2_HOLD, TOUCH_KEY_EVENT_ID_2_LOOSE},      //
    {TOUCH_KEY_ID_3, TOUCH_KEY_EVENT_ID_3_CLICK, TOUCH_KEY_EVENT_ID_3_LONG, TOUCH_KEY_EVENT_ID_3_HOLD, TOUCH_KEY_EVENT_ID_3_LOOSE},      //
    {TOUCH_KEY_ID_4, TOUCH_KEY_EVENT_ID_4_CLICK, TOUCH_KEY_EVENT_ID_4_LONG, TOUCH_KEY_EVENT_ID_4_HOLD, TOUCH_KEY_EVENT_ID_4_LOOSE},      //
    {TOUCH_KEY_ID_5, TOUCH_KEY_EVENT_ID_5_CLICK, TOUCH_KEY_EVENT_ID_5_LONG, TOUCH_KEY_EVENT_ID_5_HOLD, TOUCH_KEY_EVENT_ID_5_LOOSE},      //
    {TOUCH_KEY_ID_6, TOUCH_KEY_EVENT_ID_6_CLICK, TOUCH_KEY_EVENT_ID_6_LONG, TOUCH_KEY_EVENT_ID_6_HOLD, TOUCH_KEY_EVENT_ID_6_LOOSE},      //
    {TOUCH_KEY_ID_7, TOUCH_KEY_EVENT_ID_7_CLICK, TOUCH_KEY_EVENT_ID_7_LONG, TOUCH_KEY_EVENT_ID_7_HOLD, TOUCH_KEY_EVENT_ID_7_LOOSE},      //
    {TOUCH_KEY_ID_8, TOUCH_KEY_EVENT_ID_8_CLICK, TOUCH_KEY_EVENT_ID_8_LONG, TOUCH_KEY_EVENT_ID_8_HOLD, TOUCH_KEY_EVENT_ID_8_LOOSE},      //
    {TOUCH_KEY_ID_9, TOUCH_KEY_EVENT_ID_9_CLICK, TOUCH_KEY_EVENT_ID_9_LONG, TOUCH_KEY_EVENT_ID_9_HOLD, TOUCH_KEY_EVENT_ID_9_LOOSE},      //
    {TOUCH_KEY_ID_10, TOUCH_KEY_EVENT_ID_10_CLICK, TOUCH_KEY_EVENT_ID_10_LONG, TOUCH_KEY_EVENT_ID_10_HOLD, TOUCH_KEY_EVENT_ID_10_LOOSE}, //
};

extern u8 touch_key_get_key_id(void);
volatile struct key_driver_para touch_key_para = {
    TOUCH_KEY_SCAN_CIRCLE_TIMES,
    0,
    0,

    0,
    0,
    TOUCH_KEY_FILTER_TIMES,

    LONG_PRESS_TIME_THRESHOLD_MS / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    (LONG_PRESS_TIME_THRESHOLD_MS + HOLD_PRESS_TIME_THRESHOLD_MS) / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    0,

    0,
    0,
    200 / TOUCH_KEY_SCAN_CIRCLE_TIMES,
    // NO_KEY,
    0,
    KEY_TYPE_TOUCH,
    touch_key_get_key_id,

    TOUCH_KEY_ID_NONE,
    KEY_EVENT_NONE,
};

u8 touch_key_get_key_id(void)
{
    u8 key_val = NO_KEY;

    if (flag_is_recv_uart) // 如果串口收到了数据
    {
        if (FORMAT_HEAD_CIRCULAR_BUTTONS == (uart_recv_key_val >> 8))
        {
            // 如果是圆环按键
            key_val = TOUCH_KEY_ID_10;
        }
        else if (FORMAT_HEAD_INDEPENDENT_BUTTON == (uart_recv_key_val >> 8))
        {
            // 如果是独立的触摸按键
            // (u8)(uart_recv_key_val & 0xFF) 刚好从 0x01 ~ 0x09，对应
            // TOUCH_KEY_ID_1 ~ TOUCH_KEY_ID_9
            key_val = (u8)(uart_recv_key_val & 0xFF);
        }
        else
        {
            // 如果格式不对
            key_val = NO_KEY;
        }

        uart_recv_key_val = 0; // 清除接收到的键值
        flag_is_recv_uart = 0;
    }
    else
    {
        // 如果扫描时，未收到按键数据，认为没有检测到按键
        key_val = NO_KEY;
    }

    return key_val;
}

/**
 * @brief 将按键值和key_driver_scan得到的按键事件转换成触摸按键的事件
 *
 * @param key_val 触摸按键键值
 * @param key_event 在key_driver_scan得到的按键事件 KEY_EVENT
 * @return u8 在touch_key_event_table中找到的对应的按键事件，如果没有则返回 TOUCH_KEY_EVENT_NONE
 */
static u8 __touch_key_get_event(const u8 key_val, const u8 key_event)
{
    volatile u8 ret_key_event = TOUCH_KEY_EVENT_NONE;
    u8 i = 0;
    for (; i < ARRAY_SIZE(touch_key_event_table); i++)
    {
        // 如果往 KEY_EVENT 枚举中添加了新的按键事件，这里查表的方法就会失效，需要手动修改
        if (key_val == touch_key_event_table[i][0])
        {
            ret_key_event = touch_key_event_table[i][key_event];
            break;
        }
    }

    return ret_key_event;
}

extern str_ble_viot_para ble_viot_para;
/**
 * @brief 自定义的数据包准备和发送函数
 *
 * @param cur_key_val 当前检测到的、待发送的键值
 * @param is_hold  是否长按，0x00 -- 短按，0x01--长按
 */
void __user_ble_packet_send(u8 cur_key_val, u8 is_hold)
{
    ble_viot_para.para[0] = cur_key_val; // 存放键值
    ble_viot_para.para[1] = is_hold;     // 表示短按，在 ble_viot_encoder() 中作为数据包的数据
    ble_adv_start();
    ble_packet_send(); // 发送数据包
}

void touch_key_handle(void)
{
    u8 touch_key_event = TOUCH_KEY_EVENT_NONE;
    u8 cur_key_val = touch_key_para.latest_key_val; // 存放当前的键值

    if (touch_key_para.latest_key_val == TOUCH_KEY_ID_NONE)
    {

        return;
    }

    touch_key_event = __touch_key_get_event(touch_key_para.latest_key_val, touch_key_para.latest_key_event);
    touch_key_para.latest_key_val = TOUCH_KEY_ID_NONE;
    touch_key_para.latest_key_event = KEY_EVENT_NONE;

    switch (touch_key_event)
    {
    case TOUCH_KEY_EVENT_ID_1_CLICK:
        // my_printf("touch key 1 click\n");

        // ble_viot_para.para[0] = cur_key_val; // 存放键值
        // ble_viot_para.para[1] = 0x00;        // 表示短按，在 ble_viot_encoder() 中作为数据包的数据
        // ble_adv_start();
        // ble_packet_send(); // 发送数据包

        __user_ble_packet_send(cur_key_val, 0x00);

        break;

    case TOUCH_KEY_EVENT_ID_1_HOLD:
        // my_printf("touch key 1 hold\n");

        // ble_viot_para.para[0] = cur_key_val; // 存放键值
        // ble_viot_para.para[1] = 0x01;        // 表示长按，在 ble_viot_encoder() 中作为数据包的数据
        // ble_adv_start();
        // ble_packet_send(); // 发送数据包

        __user_ble_packet_send(cur_key_val, 0x01);

        break;

    case TOUCH_KEY_EVENT_ID_2_CLICK:
        // my_printf("touch key 2 click\n");

        __user_ble_packet_send(cur_key_val, 0x00);

        break;

    case TOUCH_KEY_EVENT_ID_2_HOLD:
        // my_printf("touch key 2 hold\n");

        __user_ble_packet_send(cur_key_val, 0x01);

        break;

    case TOUCH_KEY_EVENT_ID_3_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_3_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_4_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_4_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_5_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_5_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_6_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_6_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_7_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_7_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_8_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_8_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_9_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_9_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    case TOUCH_KEY_EVENT_ID_10_CLICK:
        __user_ble_packet_send(cur_key_val, 0x00);
        break;
    case TOUCH_KEY_EVENT_ID_10_HOLD:
        __user_ble_packet_send(cur_key_val, 0x01);
        break;

    default:
        break;
    }
}