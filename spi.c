#include <avr/io.h>
#include <stdlib.h>
#include <util/delay.h>
#include "spi.h"
#include "uart.h"
#include "aaps_a.h"

#define SPI_WAIT() while(!(SPSR & (1<<SPIF)))
#define SPI_DUMMY_BYTE 0xCC

void enable(struct hw_channel_t *hw_ch)
{
    *(hw_ch->port) |= (1<<hw_ch->cs_pin);
}
void disable(struct hw_channel_t *hw_ch)
{
    *(hw_ch->port) &= ~(1<<hw_ch->cs_pin);
}

/* TODO: Find out if this function should be static. I think it should! /PK 2013-04-28 */
uint8_t spi_transfer(uint8_t tx)
{
    SPDR = tx;
    //if(!(SPCR & (1<<SPIE)))
        SPI_WAIT();

    return SPDR;
}

uint8_t spi_send_one(struct spi_device_t *device, uint8_t buf)
{
    uint8_t recv = 0;

    init_aaps_a(device->hw_ch);
    enable(device->hw_ch);
    recv = spi_transfer(buf);
    disable(device->hw_ch);
    return recv;
}

uint8_t spi_send_multi(struct spi_device_t *dev, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    if (dev == NULL || buf == NULL)
        return -1;

//    if (dev->opto_coupled)
//        *buf = ~*buf;

    for(i = 0; i < len; i++) {
        enable(dev->hw_ch);
        buf[i] = spi_transfer(SPI_DUMMY_BYTE);
        disable(dev->hw_ch);
    }
    return 0;
}
