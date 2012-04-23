#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "uart.h"
#include "timer.h"

/*
ISR(USART_TX_vect)
{
     PORTC = 0xFF;

}*/

ISR(USART_RX_vect)
{
    char c = UDR0;
    UDR0 = c;

}
/*
 * Static function declarations
 */

/*
 * Public function declarations
 */
int kprint (const char *fmt, ...)
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
    kprint("kprint error! Output too long.\n");
    return -1;
}

void uart_init(void)
{
    /*
     *  If interrupt is enabled an interrupt vector must be present!
     */
    UBRR0L = 8;
    UCSR0B = (1<<RXEN0)|(1<<TXEN0)| (1<<RXCIE0);/* | (1<<TXCIE0)*/

    print("\x1B[2J");
    print("\x1B[0;0H");
    kprint("uart0 initialized to 57600 bps\n");
}

void print(char *c)
{
    while(*c)
    {
      /* Wait for hardware */
      while ( !( UCSR0A & (1<<UDRE0)) )
        ;
      /* Put data into buffer, sends the data */
      UDR0 = *c;
      c++;
    }
}
