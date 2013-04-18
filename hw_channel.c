#include <avr/io.h>
#include <stdlib.h>
#include "hw_channel.h"

#define CS_PIN 2
#define CS_DDR DDRE
#define CS_PORT PORTE
#define CS_LOW() (PORTE &= ~(1<<CS_PIN))
#define CS_HIGH() (PORTE |= (1<<CS_PIN))

struct hw_channel_t system_channels[2];

static void futur_enable(void)
{
    CS_LOW();
}

static void futur_disable(void)
{
    CS_HIGH();
}

void hw_init(void)
{
    struct hw_channel_t *futur = NULL;

    futur = malloc(sizeof(struct hw_channel_t)); 
    
    if (futur == NULL)
        while(1);

    system_channels[0] = *futur;
    system_channels[0].port = PORTE;
    system_channels[0].ddr = DDRE;
    system_channels[0].cs_pin = 2; 
    system_channels[0].enable = futur_enable; 
    system_channels[0].disable = futur_disable; 
}

    
