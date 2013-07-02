#include <avr/io.h>
#include <stdio.h>
#include "m128_hal.h"
#include "aaps_a.h"
#include "spi.h"
#include "uart.h"

static void disable_aaps_a(struct hw_channel_t *ch)
{
    if(ch->opto)
    {
        *(ch->port) &= ~(1 << ch->cs_pin);
    }
    else
    {
        *(ch->port) |= (1 << ch->cs_pin);
    }
}

void init_aaps_a(struct hw_channel_t *ch)
{
    /* Set MOSI, CS and SCK output */
    DDRB |= (1<<MOSI) | (1<<SCK);
    DDRB |= (1<<PB0);

    /* Set CS pin as output */
    *(ch->ddr) |= (1<<ch->cs_pin);

    /*
     * Enable SPI, Master, set clock rate.
     */
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1);
    //SPSR = (1<<SPI2X);
    disable_aaps_a(ch);
}
