#ifndef CMD_H__
#define CMD_H__

#include "spi.h"
#define CMD_INPUT_BUFFER_SIZE 32

typedef struct {
    char buffer[CMD_INPUT_BUFFER_SIZE];
    uint8_t pos;
} cmd_input_t;

void cmd_init(void);

void pending_cmd(void);
int voltage(uint16_t voltage, struct spi_device_t *dev);

#endif
