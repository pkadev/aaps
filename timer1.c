#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <timer1.h>
#include <avr/interrupt.h>

#define TIMER_LOAD_VALUE    0xFFC1 //Applicable for 16MHz
//#define TIMER_LOAD_VALUE    0xFFE1 //Applicable for 8MHz
#define NUM_TIMERS          6      //Change this if more timers are needed

struct timer_event_t
{
    int (*callback_function)();
    uint32_t delay_ms;
    uint32_t last_triggered;
    enum timer_event_type_t type;
};

static uint32_t system_time_ms = 0;
static struct timer_event_t timer_event_list[NUM_TIMERS];

static void set_timer_register(const unsigned int delay_ms)
{
    TCNT1H = delay_ms >> 8;
    TCNT1L = delay_ms & 0xFF;
}

ISR(TIMER1_OVF_vect)
{
    uint8_t i;
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
    set_timer_register(TIMER_LOAD_VALUE);
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
    for (i = 0; i < NUM_TIMERS; i++)
    {
        if (timer_event_list[i].callback_function == NULL)
        {
            timer_event_list[i].callback_function = event.callback_function;
            timer_event_list[i].delay_ms = event.delay_ms;
            timer_event_list[i].type = event.type;
            timer_event_list[i].last_triggered = system_time_ms +
                                                 event.delay_ms +
                                                 event.last_triggered;

            return true;
        }
    }
    return false;
}

bool timer1_create_timer(int (*cb_function)(), uint32_t delay_ms,
                         enum timer_event_type_t type, uint32_t offset)
{
    struct timer_event_t timer_event =
    {
        .callback_function = cb_function,
        .delay_ms = delay_ms,
        .last_triggered = offset,
        .type = type,
    };

    if (cb_function == NULL)
        return false;

    if (!register_cb(timer_event)) {
        return false;
    }

    return true;
}
