#include "fan.h"
#include "uart.h"

#define FAN0_DDR        (DDRB)
#define FAN0_PORT       (PORTB)
#define FAN0_PIN        (PB4)

#define FAN1_DDR        (DDRH)
#define FAN1_PORT       (PORTH)
#define FAN1_PIN        (PH6)

void fan_init(void)
{
    FAN0_DDR |= (1<<FAN0_PIN);
    FAN1_DDR |= (1<<FAN1_PIN);
    /* FAN0 / Counter2 */
    TCCR2A |= (1<<COM2A1) |(1<<WGM21) | (1<<WGM20);
    TCCR2B |= (1<<CS22) | (1<<CS21) |  (1<<CS20);
    //TIMSK2 |= (1<<OCIE2A);

    /** FAN1 / Counter2 */
    TCCR2A |= (1<<COM2B1);
    //TIMSK2 |= (1<<OCIE2B);
}

void fan_off(int fan)
{
    //printk("Fan%d off\n\r", fan);
    if(fan == SYS_FAN0)
    {
        OCR2A = 0;
    }
    else if(fan == SYS_FAN1)
    {
        OCR2B = 0;
    }
}

void set_fan_speed(int fan, uint16_t speed)
{
    if(fan == SYS_FAN0)
    {
        OCR2A = speed;
    }
    else if(fan == SYS_FAN1)
    {
        printk("Sysfan1 speed %u\n", speed);
        OCR2B = speed;
    }
}
