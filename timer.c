#include <stdio.h>
#include <timer.h>
#include <avr/interrupt.h>
#include "uart.h"

volatile uint16_t sys_time = 0;
volatile uint16_t sys_time_sec_low = 0;
//volatile uint8_t sys_time_sec_high = 0;

char time[SYSTIME_STR_LEN];

ISR(TIMER0_OVF_vect)
{
    TCNT0 = 4;
    sys_time++;
    if (sys_time == 1000)
    {
        sys_time_sec_low++;
        sys_time = 0;
    }
}

void timer_init(void)
{
    TCNT0 = 223;
    TIMSK0 = (1<<TOIE0);
    TCCR0B = (1<<CS01) | (1<<CS00);
    //ASSR = (1<<AS0);
    printk("System timer initilized with 1ms resolution\n");
}

static uint16_t get_sys_time_ms(void)
{
    return sys_time % 1000;
}

static uint16_t get_sys_time_s(void)
{
    return sys_time_sec_low;
}

char * get_timestamp_str()
{
    sprintf(time, "[%6u.%03u] ", get_sys_time_s(), get_sys_time_ms());
    return time;
}

