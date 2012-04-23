#include <stdio.h>
#include <timer.h>
#include <avr/interrupt.h>
#include "uart.h"

volatile uint16_t sys_time = 0;
volatile uint16_t sys_time_sec_low = 0;
//volatile uint8_t sys_time_sec_high = 0;

char time[SYSTIME_STR_LEN];

ISR(TIMER2_OVF_vect)
{
    TCNT2 = 5;
    sys_time++;
    if (sys_time == 1000)
    {
        sys_time_sec_low++;
//      if (sys_time_sec_low == 255)
//      {
//          sys_time_sec_high++;
//      }
        sys_time = 0;
    }
}

void timer_init(void)
{
    TCNT2 = 223;
    TIMSK2 = (1<<TOIE2);
    TCCR2B = (1<<CS20) | (1<<CS21);
    //ASSR = (1<<AS2);
    kprint("System timer initilized with 1ms resolution\n");
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

