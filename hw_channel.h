#ifndef HW_CHANNEL_H__
#define HW_CHANNEL_H__

struct hw_channel_t
{
    volatile uint8_t port; //Probably not needed
    volatile uint8_t ddr; //Probably not needed
    uint8_t cs_pin; //Probably not needed
    void (*enable)(void);
    void (*disable)(void);
};

void hw_init(void);

extern 
struct hw_channel_t system_channels[];
#endif

