#ifndef TIMER_H__
#define TIMER_H__

/*
 *	Global defines for driver
 */
#define SYSTIME_STR_LEN 13

/*
 * 	initializes timer
 */
void timer_init(void);

/*
 * 	Returns system time string
 */
char * get_timestamp_str(void);

#endif /* TIMER_H__ */
