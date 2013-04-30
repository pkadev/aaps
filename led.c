#include <avr/io.h>
#include "led.h"

#define LED0_DDR DDRH
#define LED1_DDR DDRL
#define LED2_DDR LED1_DDR

#define LED0_PIN 7
#define LED1_PIN 0
#define LED2_PIN 1

#define LED0_PORT PORTH
#define LED1_PORT PORTL
#define LED2_PORT LED1_PORT

void led_init(void)
{
    LED0_DDR |= (1<<LED0_PIN);
    LED1_DDR |= (1<<LED1_PIN);
    LED2_DDR |= (1<<LED2_PIN);
    led_ctrl(LED0, LED_OFF);
    led_ctrl(LED1, LED_OFF);
    led_ctrl(LED2, LED_OFF);
}

void led_ctrl(enum led_t led, enum led_state_t state)
{
    if (state == LED_OFF)
    {
        switch(led)
        {
            case LED0: LED0_PORT |= (1<<LED0_PIN); return;
            case LED1: LED1_PORT |= (1<<LED1_PIN); return;
            case LED2: LED2_PORT |= (1<<LED2_PIN); return;
        }
    }
    else if (state == LED_ON)
    {
        switch(led)
        {
            case LED0: LED0_PORT &= ~(1<<LED0_PIN); return;
            case LED1: LED1_PORT &= ~(1<<LED1_PIN); return;
            case LED2: LED2_PORT &= ~(1<<LED2_PIN); return;
        }
    }
}

