#ifndef HW_CHANNEL_H__
#define HW_CHANNEL_H__

#include "stdbool.h"

struct hw_channel_t
{
    volatile uint8_t *port; //Probably not needed
    volatile uint8_t *ddr; //Probably not needed
    uint8_t cs_pin; //Probably not needed
    bool opto;
};

void hw_init(void);
#define HW_NBR_OF_CHANNELS  14
#define NO_IRQ 0xFF
extern struct hw_channel_t *system_channel[HW_NBR_OF_CHANNELS];
extern volatile uint8_t irq_from_slave;
#endif
