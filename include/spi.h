#ifndef SPI_H__
#define SPI_H__
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "hw_channel.h"

struct spi_device_t
{
    uint8_t speed;
    bool opto_coupled;
    void (*init)(void);
    uint8_t (*transfer)(uint8_t *buf, size_t len);
    struct hw_channel_t *hw_ch;
};

uint8_t spi_transfer(uint8_t tx);
uint8_t spi_send_one(struct spi_device_t *slave, uint8_t buf);
uint8_t spi_send_multi(struct spi_device_t *dev, uint8_t *buf, uint8_t len);

#endif
