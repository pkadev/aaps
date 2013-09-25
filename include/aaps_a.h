#ifndef AAPS_A_H__
#define AAPS_A_H__
#include "hw_channel.h"

void init_aaps_a(struct hw_channel_t *ch);
uint8_t aaps_a_transfer(uint8_t *buf, size_t len);

struct spi_device_t *channel_lookup(uint8_t ch);

#endif
