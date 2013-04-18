#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <timer1.h>
#include <avr/interrupt.h>
#include "uart.h"

#define TIMER_LOAD_VALUE 0xFFC0
#define NUM_TIMERS      4

static uint16_t system_time_ms = 0;
struct timer_event_t
{
    int (*callback_function)();
    unsigned int delay_ms;
    unsigned int last_triggered;
    enum timer_event_type_t type;
};

static struct timer_event_t timer_event_list[NUM_TIMERS];

static void set_timer_register(const unsigned int delay_ms)
{
    TCNT1H = delay_ms >> 8;
    TCNT1L = delay_ms & 0xFF;
}

ISR(TIMER1_OVF_vect)
{
    uint8_t i;
    set_timer_register(TIMER_LOAD_VALUE);
    system_time_ms++;
    for (i = 0; i < NUM_TIMERS; i++) {
        if (timer_event_list[i].callback_function != 0 &&
            timer_event_list[i].last_triggered <= system_time_ms) {
            timer_event_list[i].callback_function();
            timer_event_list[i].last_triggered = system_time_ms;
            if (timer_event_list[i].type == ONE_SHOT)
                timer_event_list[i].callback_function = 0;
            else
                timer_event_list[i].last_triggered = system_time_ms +
                    timer_event_list[i].delay_ms;
        }
    }
}

void timer1_init()
{
    set_timer_register(TIMER_LOAD_VALUE);

    TIMSK1 = (1<<TOIE1);
    TCCR1B = (1<<CS12);
}

static bool register_cb(const struct timer_event_t event)
{
    uint8_t i;
    for (i = 0; i < NUM_TIMERS; i++) {
        if (timer_event_list[i].callback_function == NULL) {
            timer_event_list[i].callback_function = event.callback_function;
            timer_event_list[i].delay_ms = event.delay_ms;
            timer_event_list[i].last_triggered = system_time_ms + event.delay_ms;
            timer_event_list[i].type = event.type;

            return true;
        }
    }
    return false;
}

bool timer1_create_timer(int (*cb_function)(), unsigned int delay_ms,
                         enum timer_event_type_t type)
{
    struct timer_event_t timer_event =
    {
        .callback_function = cb_function,
        .delay_ms = delay_ms,
        .last_triggered = 0,
        .type = type,
    };

    if (cb_function == NULL)
        return false;

    if (!register_cb(timer_event)) {
        printk("error registering timer callback\n");
        return false;
    }

    return true;
}
