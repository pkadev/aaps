#include <avr/io.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "hw_channel.h"
#include "uart.h"

#define IRQ_CH00 INT7_vect
#define IRQ_CH01 INT6_vect
#define IRQ_CH02 INT5_vect
#define IRQ_CH03 INT4_vect
#define IRQ_CH04_CH05_CH06_CH07 PCINT2_vect
#define IRQ_CH08 INT0_vect
#define IRQ_CH09 INT1_vect
#define IRQ_CH10 INT2_vect
#define IRQ_CH11 INT3_vect
#define IRQ_CH12_CH13 PCINT1_vect
volatile int8_t irq_from_slave[HW_NBR_OF_CHANNELS] = {0}; //No channel

//CON11
ISR(IRQ_CH00)
{
    //printk("irq - ch1\n");
    irq_from_slave[0]++;
}

//CON12
ISR(IRQ_CH01)
{
    //printk("irq - ch1\n");
    irq_from_slave[1]++;
}

ISR(IRQ_CH02)
{
    //printk("irq - INT2\n");
    irq_from_slave[2]++;
}

ISR(IRQ_CH03)
{
    //printk("irq - INT3\n");
    irq_from_slave[3]++;
}

//CON7
ISR(IRQ_CH04_CH05_CH06_CH07)
{
    //if (!(PINK & (1<<PK1)))
    //{
    //    irq_from_slave[4]++;
    //    printk("IRQ from CH04\n");
    //}

    //if (!(PINK & (1<<PK2)))
    //{
        irq_from_slave[5]++;
        //printk("IRQ from CH05\n");
    //}

}
//CON15
ISR(IRQ_CH09)
{
    //printk("irq - CH09\n");
    irq_from_slave[9]++;
}

//CON16
ISR(IRQ_CH10)
{
    //printk("irq - CH10\n");
    irq_from_slave[10]++;
}

//CON17
ISR(IRQ_CH11)
{
    //printk("irq - CH11\n");
    irq_from_slave[11]++;
}

ISR(IRQ_CH08)
{
    //printk("irq - CH08 [%u]\n", irq_from_slave[0]);
    irq_from_slave[8]++; //Set position in system_channels[]
}

//CON4 CON18
ISR(IRQ_CH12_CH13)
{
    if (!(PINJ & (1<<PJ1)))
    {
        irq_from_slave[12]++;
        //printk("IRQ from CH12\n");
    }

    if (!(PINJ & (1<<PJ2)))
    {
        irq_from_slave[13]++;
        //printk("IRQ from CH13\n");
    }
}

void enable_extint(struct hw_channel_t *hw_ch)
{
    EIMSK |= (1 << hw_ch->irq_num);
    if (hw_ch->irq_num == INT0 || hw_ch->irq_num == INT1 ||
        hw_ch->irq_num == INT2 || hw_ch->irq_num == INT3)
    {
        printk("EICRA\n");
        EICRA |= (1 << hw_ch->irq_trig_conf);
    }
    else
    {
        printk("EICRB\n");
        EICRB |= (1 << hw_ch->irq_trig_conf);
    }
}

void enable_pcint(struct hw_channel_t *hw_ch)
{
    PCICR |= (1 << hw_ch->irq_num);

    PORTK = 0b11111011;
    if(hw_ch->irq_num == PCIE1)
    {
        PCMSK1 |= (1 << hw_ch->irq_trig_conf);
        printk("PCMSK1\n");
    }
    else if (hw_ch->irq_num == PCIE2)
    {
        PCMSK2 |= (1 << hw_ch->irq_trig_conf);
        printk("PCMSK2\n");
    }
}

//CON11
struct hw_channel_t hw_ch00 =
{
    .port = &PORTE,
    .ddr = &DDRE,
    .cs_pin = 3,
    .opto = true,
    .irq_num = 7, //INT7
    .irq_trig_conf = ISC71,
    .enable_irq = enable_extint,
};

//CON12
struct hw_channel_t hw_ch01 =
{
    .port = &PORTE,
    .ddr = &DDRE,
    .cs_pin = 2,
    .opto = true,
    .irq_num = INT6,
    .irq_trig_conf = ISC61,
    .enable_irq = enable_extint,
};

//CON10
struct hw_channel_t hw_ch03 =
{
    .port = &PORTE,
    .ddr = &DDRE,
    .cs_pin = 0,
    .opto = true,
    .irq_num = INT4,
    .irq_trig_conf = ISC41,
    .enable_irq = enable_extint,

};

//CON7
struct hw_channel_t hw_ch04 =
{
    .port = &PORTF,
    .ddr = &DDRF,
    .cs_pin = 0,
    .opto = true,
    .irq_num = PCIE2,
    .irq_trig_conf = PCINT17,
    .enable_irq = enable_pcint,

};

//CON8
struct hw_channel_t hw_ch05 =
{
    .port = &PORTF,
    .ddr = &DDRF,
    .cs_pin = 1,
    .opto = true,
    .irq_num = PCIE2,
    .irq_trig_conf = PCINT18,
    .enable_irq = enable_pcint,

};

//CON13
struct hw_channel_t hw_ch08 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 4,
    .opto = true,
    .irq_num = 0, //INT0
    .irq_trig_conf = ISC01,
    .enable_irq = enable_extint,
};

//CON16
struct hw_channel_t hw_ch10 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 6,
    .opto = true,
    .irq_num = 2, //INT2
    .irq_trig_conf = ISC21,
    .enable_irq = enable_extint,
};

//CON17
struct hw_channel_t hw_ch11 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 7,
    .opto = true,
    .irq_num = INT3,
    .irq_trig_conf = ISC31,
    .enable_irq = enable_extint,
};

//CON4
struct hw_channel_t hw_ch12 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 3,
    .opto = true,
    .irq_num = PCIE1,
    .irq_trig_conf = PCINT12,
    .enable_irq = enable_pcint,

};

//CON18
struct hw_channel_t hw_ch13 =
{
    .port = &PORTD,
    .ddr = &DDRD,
    .cs_pin = 4,
    .opto = true,
    .irq_num = PCIE1,
    .irq_trig_conf = PCINT13,
    .enable_irq = enable_pcint,
};

//CON09
struct hw_channel_t hw_ch02 =
{
    .port = &PORTE,
    .ddr = &DDRE,
    .cs_pin = 1,
    .opto = true,
    .irq_num = 5, //INT5
    .irq_trig_conf = ISC51,
    .enable_irq = enable_extint,
};

//CON15
struct hw_channel_t hw_ch09 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 5,
    .opto = true,
    .irq_num = INT1,
    .irq_trig_conf = ISC11,
    .enable_irq = enable_extint,
};

/* Make this dynamic memory allocation */
struct hw_channel_t *system_channel[HW_NBR_OF_CHANNELS] =
{
    &hw_ch00,
    &hw_ch01,
    &hw_ch02,
    &hw_ch03,
    &hw_ch04,   //PCINT
    &hw_ch05,   //PCINT
    NULL,       //PCINT
    NULL,       //PCINT
    &hw_ch08,
    &hw_ch09,
    &hw_ch10,
    &hw_ch11,
    &hw_ch12, //PCINT
    &hw_ch13, //PCINT
};



