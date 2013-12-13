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
static int current(uint16_t current, struct spi_device_t *dev);
static int reboot(void);
static int fan0_speed(uint16_t speed);
static int fan1_speed(uint16_t speed);
static int set_relay_d(uint16_t enable, struct spi_device_t *dev);

#define CHAR_BACKSPACE 0x7F

struct cmd_list_t {
    const char *name;
    int (*func)();
};

#define ASCII_CR 0x0D

ISR(USART2_RX_vect)
{
    char c = UART_DATA_REG;
    if(isalnum(c) || c == ASCII_CR || c == ' ')
    {
        if (c == ASCII_CR) {
            c = 0x00;
        }

        /*
         * This checks for input buffer overflow.
         * If this scenario appears we need to change
         * the size of CMD_INPUT_BUFFER_SIZE-
         */
        if (cmd_input.pos == (CMD_INPUT_BUFFER_SIZE)) {
            cmd_input.pos--;
            if (c == ASCII_CR)
                c = 0x00;
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
    { "gettemp", get_aaps_a_temp },
    { "getadc", get_adc },
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
    ipc_ret_t res;
    uint8_t total_len = 5;
    uint8_t payload_len  = total_len - IPC_PKT_OVERHEAD;

    struct ipc_packet_t pkt =
    {
        .len = total_len,
        .cmd = IPC_CMD_SET_CURRENT_LIMIT,
    };

    dac_current_limit = current;
    pkt.data = malloc(payload_len);
    if (pkt.data == NULL)
        printk("malloc4 failed\n");
    
    pkt.data[0] = current & 0xff;
    pkt.data[1] = (current >> 8) & 0xff;
    pkt.crc = crc8(pkt.data, payload_len);

    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);

    free(pkt.data);
    dev = NULL;
    return 0;
}

int voltage(uint16_t voltage, struct spi_device_t *dev)
{
    //printk("Voltage %u\n", dac_voltage);
    ipc_ret_t res;
    uint8_t total_len = 5;
    uint8_t payload_len  = total_len - IPC_PKT_OVERHEAD;
    struct ipc_packet_t pkt =
    {
        .len = total_len,
        .cmd = IPC_CMD_SET_VOLTAGE,
    };

    dac_voltage = voltage;

    pkt.data = malloc(payload_len);
    if (pkt.data == NULL)
        printk("malloc6 failed\n");
    
    pkt.data[0] = voltage & 0xff;
    pkt.data[1] = (voltage >> 8) & 0xff;

    pkt.crc = crc8(pkt.data, payload_len);

    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);

    free(pkt.data);
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
    ipc_ret_t res;
    struct ipc_packet_t pkt =
    {
        .len = 4,
        .cmd = IPC_CMD_SET_RELAY_D,
    };
    pkt.data = malloc(1);
    if (pkt.data == NULL)
        printk("malloc9 failed\n");
    pkt.data[0] = enable ? 1 : 0;
    pkt.crc = crc8(pkt.data, 1);
    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);
    free(pkt.data);
    dev = NULL;
    return 0;
}

int set_relay(uint16_t enable, struct spi_device_t *dev)
{

    ipc_ret_t res;
    struct ipc_packet_t pkt =
    {
        .len = 4,
        .cmd = IPC_CMD_SET_RELAY,
    };
    pkt.data = malloc(1);
    if (pkt.data == NULL)
        printk("malloc99 failed\n");
    pkt.data[0] = enable ? 1 : 0;
    pkt.crc = crc8(pkt.data, 1);
    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);
    send_set_led(IPC_LED_GREEN, pkt.data[0]);
    free(pkt.data);
    dev = NULL;
    return 0;
}

int get_aaps_a_temp(uint16_t channel, struct spi_device_t *dev)
{
    ipc_ret_t res;
    uint8_t payload_len = 1; 
    uint8_t total_len = payload_len + IPC_PKT_OVERHEAD;

    struct ipc_packet_t pkt =
    {
          .len = total_len,
          .cmd = IPC_CMD_GET_TEMP,
    };

    pkt.data = malloc(payload_len);
    if (pkt.data == NULL)
        printk("malloc11 failed\n");
    
    pkt.data[0] = channel & 0xff;
    //printk("Temp sensor: %u\n", pkt.data[0]);
    pkt.crc = crc8(pkt.data, payload_len);

    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);

    free(pkt.data);
    dev = NULL;
    return 0;
}

int get_adc(uint16_t channel, struct spi_device_t *dev)
{
    ipc_ret_t res;
    uint8_t payload_len = 1; 
    uint8_t total_len = payload_len + IPC_PKT_OVERHEAD;

    struct ipc_packet_t pkt =
    {
          .len = total_len,
          .cmd = IPC_CMD_GET_ADC,
    };

    pkt.data = malloc(payload_len);
    if (pkt.data == NULL)
        printk("malloc2 failed\n");
    
    pkt.data[0] = channel & 0xff;
    pkt.crc = crc8(pkt.data, payload_len);

    res = ipc_put_pkt(1, &pkt);
    if (res != IPC_RET_OK)
        printk("put packet failed [%u]\n", res);

    free(pkt.data);
    dev = NULL;
    return 0;
}
