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
int voltage(uint32_t voltage, struct spi_device_t *dev);
int current(uint32_t current, struct spi_device_t *dev);
int get_adc(uint16_t channel, struct spi_device_t *dev);

extern uint32_t dac_current_limit;
extern uint32_t dac_voltage;
extern bool display_calculated_values;
extern bool input_calculated_values;
void send_set_led(uint8_t led, uint8_t on);
int set_relay(uint16_t enable, struct spi_device_t *dev);
#endif
