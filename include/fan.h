#ifndef FAN_CONTROLLER_H__
#define FAN_CONTROLLER_H__
#include <avr/io.h>

enum SYSTEM_FANS {
     SYS_FAN0,
     SYS_FAN1
};

void fan_init(void);
//void test_fan(int fan);
void fan_off(int fan);
void set_fan_speed(int fan, uint16_t speed);

#endif
