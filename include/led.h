#ifndef LED_H__
#define LED_H__

enum led_t {
    LED0,
    LED1,
    LED2,
};

enum led_state_t {
    LED_ON,
    LED_OFF,
};

void led_init(void);
void led_ctrl(enum led_t led, enum led_state_t state);

#endif
