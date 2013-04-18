#ifndef SPI_H__
#define SPI_H__
#include <stdint.h>
#include <stdbool.h>
#include "hw_channel.h"

struct spi_device_t
{
    uint8_t cs_pin;
    uint8_t cs_port;
    uint8_t speed;
    void (*enable)(void); 
    void (*disable)(void); 
    struct hw_channel_t *hw_ch;
};

void spi_init(struct spi_device_t *dev);
uint8_t spi_transfer(uint8_t tx);
uint8_t spi_send_one(struct spi_device_t *slave, uint8_t buf);
uint8_t spi_send_multi(struct spi_device_t *dev, char *buf, uint8_t len);

#endif
