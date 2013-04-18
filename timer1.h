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
 * 	initializes timer
 */
void timer1_init();

bool timer1_create_timer(int (*cb_function)(), unsigned int delay_ms,
                         enum timer_event_type_t type);

/*
 * 	Returns system time string
 */

char * get_timestamp_str(void);


#endif /* TIMER_H__ */
