/* ---------------------------------------------------------------
** File name:		uart.c
** Brief:
** COPYRIGHT(C) ZXW 2020
** ---------------------------------------------------------------
** Module Information:
**
** Revision History ----------------------------------------------
** ---------------------------------------------------------------
**  Date        Version     Action                  Author
** ---------------------------------------------------------------
**  2019-5-27   1.0         Created
**
** ---------------------------------------------------------------
*/

/* Includes -----------------------------------------------------*/
#include "type.h"
#include "config.h"
#include "uart.h"
#include "sys.h"

// #include <stdio.h>

// #include "user_config.h"

volatile u8 uart_recv_buf[20] = {0};
volatile u16 uart_recv_key_val = 0; // 存放接收到的按键键值

#if 1 // 将 uart 用作串口打印
// 重写putchar()函数
extern void uart_putc(uint8_t c); // 函数声明

// #include <stdio.h>
#include <stdarg.h>
#include "uart.h"

/* Macro --------------------------------------------------------*/

#define putchar     uart_putc

#ifdef fputc
#undef fputc
#define fputc uart_putc
#endif
 

/* Global Variables ---------------------------------------------*/

/* Extern -------------------------------------------------------*/

/* Functions ----------------------------------------------------*/

void my_printf(char *s, ...);
void printNum(unsigned long num, int base);
void printDeci(int dec);
void printOct(unsigned oct);
void printHex(unsigned hex);
void printAddr(unsigned long addr);
void printStr(char *str);
void printFloat(double f);

// extern char fputc(char c);
// int fputc(char c)
// int putchar(int ch)
// {
//     while ((read_reg(UART_STATE_REG) & UART_TX_BUF_FULL_MASK))
//         ;
//     write_reg(BB_UART_DATA_REG, ch);
//     return ch;
// }

void my_printf(char *s, ...)
{
    int i = 0;

    va_list va_ptr;
    va_start(va_ptr, s);

    while (s[i] != '\0')
    {
        if (s[i] != '%')
        {
            putchar(s[i++]);
            continue;
        }

        switch (s[++i])
        {
        case 'd':
            printDeci(va_arg(va_ptr, int));
            break;
        case 'o':
            // printOct(va_arg(va_ptr, unsigned int));
            break;
        case 'x':
            printHex(va_arg(va_ptr, unsigned int));
            break;
        case 'c':
            putchar(va_arg(va_ptr, int));
            break;
        case 'p':
            // printAddr(va_arg(va_ptr, unsigned long));
            break;
        case 'f':
            // printFloat(va_arg(va_ptr, double));
            break;
        case 's':
            printStr(va_arg(va_ptr, char *));
            break;
        default:
            break;
        }

        i++;
    }

    va_end(va_ptr);
}

void printNum(unsigned long num, int base)
{
    if (num == 0)
    {
        return;
    }

    printNum(num / base, base);

    putchar("0123456789abcdef"[num % base]);
}

void printDeci(int dec)
{
    //    int num;

    if (dec < 0)
    {
        putchar('-');
        dec = -dec;
    }

    if (dec == 0)
    {
        putchar('0');
        return;
    }
    else
    {
        printNum(dec, 10);
    }
}

// void printOct(unsigned oct)
// {
//     if (oct == 0)
//     {
//         putchar('0');
//         return;
//     }
//     else
//     {
//         printNum(oct, 8);
//     }
// }

void printHex(unsigned hex)
{
    if (hex == 0)
    {
        putchar('0');
        return;
    }
    else
    {
        printNum(hex, 16);
    }
}

// void printAddr(unsigned long addr)
// {
//     putchar('0');
//     putchar('x');

//     printNum(addr, 16);
// }

void printStr(char *str)
{
    int i = 0;

    while (str[i] != '\0')
    {
        putchar(str[i++]);
    }
}

// void printFloat(double f)
// {
//     int temp;

//     temp = (int)f;
//     printNum(temp, 10);

//     putchar('.');

//     f -= temp;
//     if (f == 0)
//     {
//         for (temp = 0; temp < 6; temp++)
//         {
//             putchar('0');
//         }
//         return;
//     }
//     else
//     {
//         temp = (int)(f * 1000000);
//         printNum(temp, 10);
//     }
// }
#endif // 将 uart 用作串口打印

void uart_init(uart_baud_rate_t baud_rate_index)
{
    uint32_t reg_val = 0;

    /* Clock enable */
    sys_set_module_clock(CLK_UART_EN_MASK, ON);

    /* Reset UART module */
    sys_soft_reset(UART_SOFT_RESET_MASK);

    /*cpu clk*/
    reg_val = 0;
    reg_val = read_reg(TOP_MODULE_MODE_REG);
    reg_val &= TOP_CPU_CLK_SEL_MASK;
    reg_val = reg_val >> TOP_CPU_CLK_SEL_SHIFT;

    /*divisor = freq / baudrate,  freq = 3000000<<reg_val ,divisor>=16*/
#if 0

    reg_val = 3000000*(0x1<<reg_val)/baudrate;

#else
    uint32_t quotient = 0x0;
    reg_val = (3000000 << reg_val);

    while (1)
    {
        if (reg_val >= baud_rate_index)
        {
            quotient++;
            reg_val -= baud_rate_index;
        }
        else
        {
            break;
        }
    }

    reg_val = quotient;
#endif

    write_reg(UART_BAUD_DIV_REG, reg_val);
    reg_val = 0;

    /* Enable Rx, Tx & Rx interrupt */
    reg_val = UART_RX_EN_MASK | UART_TX_EN_MASK | UART_RX_INT_EN_MASK;
    write_reg(UART_CTRL_REG, reg_val);
}

void uart_set_baudrate(uart_baud_rate_t baud_rate_index)
{
    uint32_t reg_val = 0;
    /*cpu clk*/
    reg_val = 0;
    reg_val = read_reg(TOP_MODULE_MODE_REG);
    reg_val &= TOP_CPU_CLK_SEL_MASK;
    reg_val = reg_val >> TOP_CPU_CLK_SEL_SHIFT;

    if (!reg_val)
    {
        return;
    }
    /* Baud rate */
    if ((baud_rate_index >= 2400) && (baud_rate_index <= 600000))
    {
    }
    else
    {
        baud_rate_index = 115200;
    }
    /*divisor = freq / baudrate,  freq = 3000000<<reg_val ,divisor>=16*/
#if 0

    reg_val = 3000000*(0x1<<reg_val)/baudrate;

#else
    uint32_t quotient = 0x0;
    reg_val = (3000000 << reg_val);

    while (1)
    {
        if (reg_val >= baud_rate_index)
        {
            quotient++;
            reg_val -= baud_rate_index;
        }
        else
        {
            break;
        }
    }

    reg_val = quotient;
#endif

    write_reg(UART_BAUD_DIV_REG, reg_val);
}

void uart_set_port(uart_port_t tx_rx_port)
{
    switch (tx_rx_port)
    {
    case 0x0:
    {
        sys_set_port_mux(PAD_GPIO_00, PAD_MUX_FUNCTION_1 | 2); // RXD
        sys_set_port_mux(PAD_GPIO_01, PAD_MUX_FUNCTION_1 | 3); // TXD
        break;
    }
    case 0x1:
    {

        sys_set_port_mux(PAD_GPIO_08, PAD_MUX_FUNCTION_3 | 2); // RXD
        sys_set_port_mux(PAD_GPIO_01, PAD_MUX_FUNCTION_1 | 3); // TXD

        break;
    }
    case 0x2:
    {
        sys_set_port_mux(PAD_GPIO_00, PAD_MUX_FUNCTION_1 | 2); // RXD
        sys_set_port_mux(PAD_GPIO_09, PAD_MUX_FUNCTION_3 | 3); // TXD

        break;
    }
    case 0x3:
    {
        sys_set_port_mux(PAD_GPIO_08, PAD_MUX_FUNCTION_3 | 2); // RXD
        sys_set_port_mux(PAD_GPIO_09, PAD_MUX_FUNCTION_3 | 3); // TXD
        break;
    }
    default:
    {
        break;
    }
    }
}

void uart_set_recv_int(uint8_t enable_recv_int)
{
    uint32_t reg_val = 0;

    reg_val = read_reg(UART_CTRL_REG);

    if (TRUE == enable_recv_int)
    {
        reg_val = reg_val | UART_RX_INT_EN_MASK;
    }
    else
    {
        reg_val = reg_val & (~UART_RX_INT_EN_MASK);
    }

    write_reg(UART_CTRL_REG, reg_val);
}

uint8_t uart_get_rx_int_status()
{
    uint8_t reg_val = 0;

    reg_val = read_reg(UART_INT_REG);
    return (reg_val & UART_RX_INT_MASK) >> UART_RX_INT_SHIFT;
}

uint8_t uart_get_rx_status()
{
    uint8_t reg_val = 0;

    reg_val = read_reg(UART_STATE_REG);
    return (reg_val & UART_RX_BUF_FULL_MASK) >> UART_RX_BUF_FULL_SHIFT;
}

void uart_putc(uint8_t c)
{
    // uint32_t reg_val = 0;

    /* If TX buffer is empty */
    while ((read_reg(UART_STATE_REG) & UART_TX_BUF_FULL_MASK))
        ;
    write_reg(BB_UART_DATA_REG, c);

    /* Clear Tx interrupt */
    // write_reg(UART_INT_REG, UART_TX_INT_MASK);
}

uint8_t uart_getc(void)
{
    uint8_t reg_val = 0;

    reg_val = read_reg(BB_UART_DATA_REG);

    return reg_val;
}

// ATTRIBUTE_ISR ATTRIBUTE_WEAK void uart_isr()
// {
//     uint32_t reg_val = 0;
//     reg_val = read_reg(UART_INT_REG);

//     /*RX interrupt */
//     // if (UART_RX_INT_MASK & reg_val)
//     {
// #ifdef MODULE_TEST

//         uint8_t aa = uart_getc();
//         // uart_putc(aa + 1); // temp test code.
//         my_printf("recv\n");

// #endif
//     }
//     /*TX interrupt*/
//     // else if (UART_TX_INT_MASK & reg_val)
//     // {
//     // }
//     write_reg(UART_INT_REG, UART_RX_INT_MASK | UART_TX_INT_MASK);
// }
