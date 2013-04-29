#include <avr/io.h>
#include <stdlib.h>
#include "spi.h"
#include "uart.h"

#define SPI_WAIT() while(!(SPSR & (1<<SPIF)))
#define SPI_CS_HIGH(pin) (PORTE |= (1<<pin))
#define SPI_CS_LOW(pin) (PORTE &= ~(1<<pin))
#define SPI_DUMMY_BYTE 0xCC

/* TODO: Find out if this function should be static. I think it should! /PK 2013-04-28 */
uint8_t spi_transfer(uint8_t tx)
{
    SPDR = tx;
    if(!(SPCR & (1<<SPIE)))
        SPI_WAIT();

    return SPDR;
}

uint8_t spi_send_one(struct spi_device_t *device, uint8_t buf)
{
    uint8_t recv = 0;

    device->hw_ch->enable();
    recv = spi_transfer(buf);
    device->hw_ch->disable();
    return recv;
}

uint8_t spi_send_multi(struct spi_device_t *dev, char *buf, uint8_t len)
{
    uint8_t i;
    if (dev == NULL || buf == NULL)
        return -1;

    SPI_CS_LOW(dev->cs_pin);
    for(i = 0; i < len; i++) {
        buf[i] = spi_transfer(SPI_DUMMY_BYTE);
    }
    SPI_CS_HIGH(dev->cs_pin);
    return 0;
}
