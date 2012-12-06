#ifndef SETTINGS_H__
#define SETTINGS_H__
#include "stdint.h"

#define SETTINGS_ADDR 0x00

struct system_settings
{
    uint8_t num_analog_channels;
};

#define SETTINGS_VOLTAGE_ADDR 0x00

void settings_write_settings(int voltage);
void settings_print_sys_settings();
void settings_get_settings(struct system_settings *settings);

#endif
