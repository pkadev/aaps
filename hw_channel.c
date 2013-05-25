#include <avr/io.h>
#include <stdlib.h>
#include "hw_channel.h"

#define CS_PIN 3
#define CS_DDR DDRL
#define CS_PORT PORTL
#define CS_LOW() (CS_PORT &= ~(1<<CS_PIN))
#define CS_HIGH() (CS_PORT |= (1<<CS_PIN))

struct hw_channel_t system_channels[2];

static void futur_enable(void)
{
    CS_HIGH();
}

static void futur_disable(void)
{
    CS_LOW();
}

void hw_init(void)
{
    struct hw_channel_t *futur = NULL;

    futur = malloc(sizeof(struct hw_channel_t)); 
    
    if (futur == NULL)
        while(1);

    system_channels[0] = *futur;
    system_channels[0].port = PORTL;
    system_channels[0].ddr = DDRL;
    system_channels[0].cs_pin = 3; 
    system_channels[0].enable = futur_enable; 
    system_channels[0].disable = futur_disable; 
}

    
