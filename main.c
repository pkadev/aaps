#include <stdint.h>
#include "uart.h"
#include "adc.h"
#include "timer.h"
#include <util/delay.h>
#include <avr/interrupt.h>

int main(void)
{
    SREG |= (1<<SREG_I);
    uart_init();
    timer_init();

    kprint("Running CPU mega88 @ 8.0 MHz\n");
    kprint("System booted\n");

    while(1)
    {
    }
    return 0;
}

