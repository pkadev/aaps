#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "m128_hal.h"
#include "uart.h"
#include "timer.h"

/*
ISR(USART_TX_vect)
{
     PORTC = 0xFF;

}*/

/*
 * Static function declarations
 */

/*
 * Public function declarations
 */
int printk (const char *fmt, ...)
{
    va_list args;
    char printbuffer[RS232_PRINT_BUFFER] = {0};
    if (strlen(fmt) + SYSTIME_STR_LEN > RS232_PRINT_BUFFER) {
        goto error;
    }
    va_start (args, fmt);

    vsprintf (printbuffer, fmt, args);

    va_end (args);
    if (strlen(printbuffer) + SYSTIME_STR_LEN <= RS232_PRINT_BUFFER)
    {
        memmove(printbuffer+SYSTIME_STR_LEN, printbuffer, strlen(printbuffer));
        memcpy(printbuffer, get_timestamp_str(), SYSTIME_STR_LEN);
        print(printbuffer);
    }
    return 0;
error:
    printk("printk error! Output too long.\n");
    return -1;
}

void uart_init(void)
{
    /*
     *  If interrupt is enabled an interrupt vector must be present!
     */
    UART_BAUD_RATE_REG_LOW = 12;
    UCSR2B = (1<<RXEN2) | (1<<TXEN2) | (1<<RXCIE2);/* | (1<<TXCIE2)*/

    print("\x1B[2J");
    print("\x1B[0;0H");
    printk("uart0 initialized to 57600 bps\n");
}

void print(char *c)
{
    while(*c)
    {
      /* Wait for hardware */
      while ( !( UCSR2A & (1<<UDRE2)) )
        ;
      /* Put data into buffer, sends the data */
      UART_DATA_REG = *c;
      c++;
    }
}
