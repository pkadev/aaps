#include <string.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "m128_hal.h"
#include "cmd.h"
#include "uart.h"
#include "1wire.h"
#include "fan.h"
#include "settings.h"
#include "spi.h"
#include "ipc.h"


int (*pt2Function)(void) = 0;
static uint8_t initialized = 0;
static cmd_input_t cmd_input;
static void find_service(const char * service);

/*
 * Functions that can be registered
 */
static int help(void);
static int temp_out(void);
static int volt_out(void);
static int reboot(void);
static int fan_cmd_on(void);
static int fan_cmd_med(void);
static int fan_cmd_med2(void);
static int fan_cmd_off(void);
static int cmd_set_voltage(void);
static int cmd_send_ipc(void);

#define CHAR_BACKSPACE 0x7F

struct cmd_list_t {
    const char *name;
    int (*func)(void);
};

ISR(USART2_RX_vect)
{
    char c = UART_DATA_REG;
    if(isalnum(c) || c == 0x0D || c == ' ')
    {
        if (c == 0x0D) {
            c = 0x0;
        }
        cmd_input.buffer[cmd_input.pos] = c;
        cmd_input.pos++;
    } else if (c == CHAR_BACKSPACE) {
        cmd_input.pos--;
       // printk("char: %i\n", c);
    }
}

void cmd_init(void)
{
    memset(cmd_input.buffer, 0, CMD_INPUT_BUFFER_SIZE);
    cmd_input.pos = 0;
    printk("Cmd initialized\n");
    initialized = 1;
}

void pending_cmd(void)
{
    if(initialized == 1)
    {
        if (cmd_input.pos != 0 )
        {
            if (cmd_input.buffer[cmd_input.pos-1] == 0x0 && cmd_input.pos > 1)
            {
                find_service(cmd_input.buffer);

                if( pt2Function != 0) {
                    pt2Function();
                    pt2Function = 0;
                } else {
                    printk("%s not found\n", cmd_input.buffer);
                }
                memset(cmd_input.buffer, 0, CMD_INPUT_BUFFER_SIZE);
                cmd_input.pos = 0;
            }
        }
    }
}

static struct cmd_list_t cmd_list[] = {
    { "help", help },
    { "test", temp_out },
    { "voltout", volt_out },
    { "reboot", reboot },
    { "fanon", fan_cmd_on },
    { "fanmed", fan_cmd_med },
    { "fanmed2", fan_cmd_med2 },
    { "fanoff", fan_cmd_off },
    { "setvoltage", cmd_set_voltage },
    { "send", cmd_send_ipc },
};

static void find_service(const char * service)
{
    uint8_t i;
    size_t list_len = sizeof(cmd_list) / sizeof(cmd_list[0]);

    for (i = 0; i < list_len; i++)
    {
        if (strcmp(service, cmd_list[i].name) == 0)
            pt2Function = cmd_list[i].func;
    }
}

/*
 * Functions that can be registered
 */
static int help(void)
{
    uint8_t i;
    char *name;
    size_t len;
    size_t list_len = sizeof(cmd_list) / sizeof(cmd_list[0]);

    for (i = 1; i < list_len; i++)
    {
        len = strlen(cmd_list[i].name) + 1 + 1;
        name = malloc(len);

        if (name) {
            memcpy(name, cmd_list[i].name, len - 2);
            name[len-2]  = '\n';
            name[len-1]  = 0;
            //printk("Allocated %u bytes\n", len);
        }
        printk(name);
        free(name);
        name = NULL;
    }
    return 0;
}

static int volt_out(void)
{
    printk("Volt: 5.01V\n");
    return 0;
}

static int temp_out(void)
{
    ow_device_t rom;
    ow_temp_t temperature;
    if (ow_read_temperature(&rom, &temperature) == OW_RET_OK)
        printk("Ambient temperature: %u.%u°C\n",temperature.temp, temperature.dec);
    else
        printk("CRC failed\n");
    return 0;
}

static int reboot(void)
{
    printk("Rebooting...\n");
    wdt_enable(WDTO_250MS);

    /* Wait for WD reset */
    while(1)
        ;
    return 0;
}
static int fan_cmd_on(void)
{
    printk("Fan is on!\n");
    set_fan_speed(SYS_FAN0, 240);
    return 0;
}
static int fan_cmd_med(void)
{
    set_fan_speed(SYS_FAN0, 125);
    return 0;
}

static int fan_cmd_med2(void)
{
    set_fan_speed(SYS_FAN0, 195);
    return 0;
}

static int fan_cmd_off(void)
{
    printk("Fan is off!\n");
    set_fan_speed(SYS_FAN0, 0);
    return 0;
}

static int cmd_set_voltage(void)
{
    char *fw = strchr(cmd_input.buffer, ' ');
    char *bw = strrchr(cmd_input.buffer, ' ');
    char *param = bw + 1;

    if (fw != bw) {
        printk("Too many parameters\n");
        return -1;
    }

    if (!isdigit(*(fw+1))) {
        printk("Param not integer [%s]\n", (fw + 1));
        return -1;
    }

    int voltage = atoi(param);
    settings_write_settings(voltage);
    return 0;
}

static int cmd_send_ipc(void)
{
    char *fw = strchr(cmd_input.buffer, ' ');
    char *bw = strrchr(cmd_input.buffer, ' ');
    char *param = bw + 1;

    if (fw != bw) {
        printk("Too many parameters\n");
        return -1;
    }

    if (!isdigit(*(fw+1))) {
        printk("Param not integer [%s]\n", (fw + 1));
        return -1;
    }

    struct spi_device_t dev;
    dev.cs_pin = 2;
    spi_send_one(&dev, atoi(param));
    return 0;
}

