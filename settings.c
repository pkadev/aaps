#include "settings.h"
#include "uart.h"
#include <avr/eeprom.h>

void settings_print_sys_settings()
{
    printk("System settings:\n");
    printk("  Analog channels: %u\n");
}

void settings_write_settings(int voltage)
{
    if (voltage != eeprom_read_word(0)) {
        printk("set votage: %d\n", voltage);
        //eeprom_write_word(NULL, voltage);
    } else {
        printk("Already set\n");
    }
}

void settings_get_settings(struct system_settings *settings)
{
    int data = eeprom_read_word(0);
    printk("Setting: %i\n", data);
}
