#ifndef HW_TIMER1_H__
#define HW_TIMER1_H__
#include <stdbool.h>
/*
 *	Global defines for driver
 */
enum timer_event_type_t
{
    ONE_SHOT,
    PERIODIC,
};

/*
 * 	Initializes timer.
 *  Call this function before any other calls to timer1
 */
void timer1_init();

bool timer1_create_timer(int (*cb_function)(), uint32_t delay_ms,
                         enum timer_event_type_t type, uint32_t offset);

#endif /* TIMER_H__ */
