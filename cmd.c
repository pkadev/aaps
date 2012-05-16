#include <string.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <ctype.h>
#include "m128_hal.h"
#include "cmd.h"
#include "uart.h"
#include "1wire.h"

int (*pt2Function)(void) = 0;

static cmd_input_t cmd_input;
static void find_service(const char * service);

/*
 * Functions that can be registered
 */
static int temp_out(void);
static int volt_out(void);
static int reboot(void);

ISR(USART0_RX_vect)
{
    char c = UART_DATA_REG;
    if(isalnum(c) || c == 0x0D)
    {
        if (c == 0x0D) {
            c = 0x0;
        }
        cmd_input.buffer[cmd_input.pos] = c;
        cmd_input.pos++;
    }
}

void cmd_init(void)
{
    memset(cmd_input.buffer, 0, CMD_INPUT_BUFFER_SIZE);
    cmd_input.pos = 0;
    printk("Cmd initilized\n");
}

void pending_cmd(void)
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


static void find_service(const char * service)
{
    if (strcmp(service, "temp") == 0)
        pt2Function = temp_out;
    if (strcmp(service, "volt_out") == 0)
        pt2Function = volt_out;
    if (strcmp(service, "reboot") == 0)
        pt2Function = reboot;
}

/*
 * Functions that can be registered
 */
static int volt_out(void)
{
    printk("Volt: 5.01V\n");
    return 0;
}

static int temp_out(void)
{
    uint8_t rom;
    ow_temp_t temperature;
    ow_read_temperature(&rom, &temperature);
    printk("Ambient temperature: %u.%u°C\n",temperature.temp, temperature.dec);
    return 0;
}

static int reboot(void)
{
    printk("Rebooting...\n");
    WD_CTRL_REG |= (1<<WD_CHANGE_ENABLE) |(1<<WD_ENABLE);
    WD_CTRL_REG = (1<<WD_ENABLE) | (1<<WD_PRESCALER1) | (1<<WD_PRESCALER0);

    /* Wait for WD reset */
    while(1)
        ;
    return 0;
}
