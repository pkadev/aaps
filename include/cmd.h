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
int voltage(uint32_t voltage, uint8_t slave);
int current(uint32_t current, uint8_t slave);
//int get_adc(uint16_t channel, uint8_t slave);
int get_adc(uint32_t channel, uint8_t slave);

extern uint32_t dac_current_limit;
extern uint32_t dac_voltage;
extern bool display_calculated_values;
extern bool input_calculated_values;
//int get_aaps_a_temp(uint16_t channel, uint8_t slave);
int get_aaps_a_temp(uint32_t channel, uint8_t slave);
void send_set_led(uint8_t led, uint8_t on);
//int set_relay(uint16_t enable, uint8_t slave);
int set_relay(uint32_t enable, uint8_t slave);
#endif
