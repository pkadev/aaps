#include <avr/io.h>
#include <stdlib.h>
#include "hw_channel.h"

struct hw_channel_t system_channel;

struct hw_channel_t hw_ch12 =
{
    .port = &PORTL,
    .ddr = &DDRL,
    .cs_pin = 3,
};

void hw_init(void)
{
    system_channel = hw_ch12;
}
