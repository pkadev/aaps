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
#include "aaps_a.h"

int (*pt2Function)() = 0;
static uint8_t initialized = 0;
static cmd_input_t cmd_input;
static void find_service(const char * service);
uint16_t param = 0;
struct spi_device_t *dev = 0;
/*
 * Functions that can be registered
 */
static int help(void);
static int temp_out(void);
static int voltage(uint16_t voltage, struct spi_device_t *dev);
static int current(uint16_t current, struct spi_device_t *dev);
static int reboot(void);
static int fan0_speed(uint16_t speed);
static int fan1_speed(uint16_t speed);
static int set_relay_d(uint16_t enable, struct spi_device_t *dev);
static int set_relay(uint16_t enable, struct spi_device_t *dev);

#define CHAR_BACKSPACE 0x7F
#define IPC_DUMMY_CRC 0xbc

struct cmd_list_t {
    const char *name;
    int (*func)();
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
    if(initialized)
    {
        if (cmd_input.pos != 0 )
        {
            if (cmd_input.buffer[cmd_input.pos-1] == 0x0 && cmd_input.pos > 1)
            {
                find_service(cmd_input.buffer);

                if( pt2Function != 0) {
                    pt2Function(param, dev);
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
    { "voltage", voltage },
    { "current", current },
    { "reboot", reboot },
    { "fan0", fan0_speed },
    { "fan1", fan1_speed },
    { "relayd", set_relay_d },
    { "relay", set_relay },
};

static void find_service(const char * service)
{
    uint8_t i;
    size_t list_len = sizeof(cmd_list) / sizeof(cmd_list[0]);
    char *delimiter = strchr(service, ' ');

    if(delimiter) {
        param = atoi(delimiter+1);
        *delimiter = 0x00;

        /* Check if command has a channel as
         * additional parameter
         */
        delimiter = strchr(delimiter + 1, ' ');
        if(delimiter) {

            /* TODO: This lookup must be done in
             * some other way
             */
            /* TODO: This is a bug if this doesn't convert
             * to a integer within number of available
             * channels.
             */
            dev = channel_lookup(atoi(delimiter+1));
        } else {
            dev = NULL;
        }
    }

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
        } else {
            /* Out of memory */
            return -1;
        }

        printk(name);
        free(name);
        name = NULL;
    }
    return 0;
}

static int current(uint16_t current, struct spi_device_t *dev)
{
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    uint8_t ipc_packet[] =
    {
        IPC_CMD_SET_CURRENT_LIMIT, 0x02,
        current & 0xff, (current >> 8) & 0xff,
        IPC_DUMMY_CRC,
    };

    if (dev == NULL) {
        printk("NULL device. Failed to set current.\n");
        return -1;
    }
    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(ipc_packet[cnter++]));
    }
    dev = NULL;
    return 0;
}

static int voltage(uint16_t voltage, struct spi_device_t *dev)
{
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    uint32_t convert = (uint32_t)voltage * 100000;
    uint16_t ratio = 57632;

    if (dev == NULL) {
        printk("NULL device. Failed to set voltage.\n");
        return -1;
    }
    if (voltage > 33000)
        voltage = 33000;
    if (voltage == 0)
        convert = 0;

    convert /= ratio;
    voltage = convert;

    uint8_t packet1[] =
    {
        IPC_CMD_SET_VOLTAGE, 0x02,
        voltage & 0xff, (voltage >> 8) & 0xff,
        0xEF
    };

    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(packet1[cnter++]));
    }
    dev = NULL;
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

static int fan0_speed(uint16_t speed)
{
    printk("FAN0 speed: %u\n", speed);
    set_fan_speed(SYS_FAN0, speed);
    return 0;
}

static int fan1_speed(uint16_t speed)
{
    printk("FAN1 speed: %u\n", speed);
    set_fan_speed(SYS_FAN1, speed);
    return 0;
}

static int set_relay_d(uint16_t enable, struct spi_device_t *dev)
{
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    uint8_t ipc_packet[] =
    {
        IPC_CMD_SET_RELAY_D,
        0x02,
        0x00,
        0x00,
        IPC_DUMMY_CRC,
    };
    ipc_packet[3] = enable ? 1 : 0;

    if (dev == NULL) {
        printk("NULL device. Failed to set current.\n");
        return -1;
    }
    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(ipc_packet[cnter++]));
    }
    dev = NULL;
    return 0;
}

static int set_relay(uint16_t enable, struct spi_device_t *dev)
{
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    uint8_t ipc_packet[] =
    {
        IPC_CMD_SET_RELAY,
        0x02,
        0x00,
        0x00,
        IPC_DUMMY_CRC,
    };
    ipc_packet[3] = enable ? 1 : 0;

    if (dev == NULL) {
        printk("NULL device. Failed to set current.\n");
        return -1;
    }
    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(ipc_packet[cnter++]));
    }
    dev = NULL;
    return 0;
}
