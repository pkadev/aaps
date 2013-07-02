#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "hw_channel.h"
#include "uart.h"

#define IRQ_CH00 INT7_vect
#define IRQ_CH01 INT6_vect
#define IRQ_CH02 INT5_vect
#define IRQ_CH03 INT4_vect
#define IRQ_CH04 PCINT2_vect
#define IRQ_CH05 PCINT2_vect
#define IRQ_CH06 PCINT2_vect
#define IRQ_CH07 PCINT2_vect
#define IRQ_CH08 INT0_vect
#define IRQ_CH09 INT1_vect
#define IRQ_CH10 INT2_vect
#define IRQ_CH11 INT3_vect
#define IRQ_CH12 PCINT1_vect
#define IRQ_CH13 PCINT1_vect

volatile uint8_t irq_from_slave = 0;

ISR(IRQ_CH00)
{
    //printk("irq - ch1\n");
//    irq_from_slave = 1;
}

ISR(IRQ_CH01)
{
    //printk("irq - ch1\n");
//    irq_from_slave = 1;
}
ISR(IRQ_CH02)
{
    //printk("irq - INT5\n");
//    irq_from_slave = 1;
}

ISR(IRQ_CH10)
{
//    printk("irq - CH08\n");
    irq_from_slave = 1;
}
ISR(IRQ_CH08)
{
 //   printk("irq - CH10\n");
    irq_from_slave = 1;
}

ISR(IRQ_CH12)
{
    if (!(PINJ & (1<<PJ1))) {
        irq_from_slave = 1;
//        printk("IRQ from CH12\n");
    }
}

//CON13
struct hw_channel_t hw_ch03 =
{
    .port = &PORTE,
    .ddr = &DDRE,
    .cs_pin = 0,
    .opto = true,
};
//CON13
struct hw_channel_t hw_ch08 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 4,
    .opto = true,
};

//CON4
struct hw_channel_t hw_ch10 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 6,
    .opto = true,
};

struct hw_channel_t hw_ch12 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 3,
    .opto = true,
};

//CON18
struct hw_channel_t hw_ch13 =
{
    .port = &PORTD,
    .ddr = &DDRD,
    .cs_pin = 4,
    .opto = true,
};

void hw_init(void)
{
    uint8_t i;
    for(i = 0; i < HW_NBR_OF_CHANNELS; i++) {
        if (system_channel[i] != NULL)
            printk("Detecting channel %u\n", i);
    }
    /* Configure IRQ for connected channels */
//  EICRB |= (1<<ISC51) | (1<<ISC50);
    EICRA |= (1<<ISC01) | (1<<ISC21);
    EIMSK |= (1<<INT0) | (1 << INT2);
    /* End Configure IRQ */
    //system_channel = hw_ch12;
}

/* Make this dynamic memory allocation */
struct hw_channel_t *system_channel[HW_NBR_OF_CHANNELS] =
{
    &hw_ch08,
    &hw_ch10,
    NULL,
    NULL,
    NULL,
    &hw_ch03,
    &hw_ch12,
    NULL,
    &hw_ch13,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};
