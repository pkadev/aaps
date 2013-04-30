
#ifndef FAN_CONTROLLER_H__
#define FAN_CONTROLLER_H__
#include <avr/io.h>
#define FAN_DDR (DDRB)
#define FAN_PORT    (PORTB)
#define FAN0_PIN    (PB4)
#define FAN1_PIN    (PB5)
#define FAN_ON(X)   FAN_PORT|=(1<<x)
#define FAN_OFF(X)  FAN_PORT&=~(1<<x)
#define FAN0_DDR            (FAN_DDR)
#define FAN1_DDR         (DDRH)

enum SYSTEM_FANS {
     SYS_FAN0,
     SYS_FAN1
};

void fan_init(void);
//void test_fan(int fan);
void fan_off(int fan);
void set_fan_speed(int fan, uint16_t speed);

#endif
