#include <string.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <stdio.h>
#include <ctype.h>
#include "m128_hal.h"
#include "cmd.h"
#include "uart.h"
#include "1wire.h"
#include "fan.h"

int (*pt2Function)(void) = 0;
static uint8_t initialized = 0;
static cmd_input_t cmd_input;
static void find_service(const char * service);

/*
 * Functions that can be registered
 */
static int temp_out(void);
static int volt_out(void);
static int reboot(void);
static int fan_cmd_on(void);
static int fan_cmd_med(void);
static int fan_cmd_med2(void);
static int fan_cmd_off(void);

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

void ow_print_device_addr(ow_device_t *ow_device)
{
        printk("%02X %02X %02X %02X %02X %02X %02X %02X \n",
               ow_device->addr[7], ow_device->addr[6],
               ow_device->addr[5], ow_device->addr[4], 
               ow_device->addr[3], ow_device->addr[2],
               ow_device->addr[1], ow_device->addr[0]); 
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


static void find_service(const char * service)
{
    if (strcmp(service, "temp") == 0)
        pt2Function = temp_out;
    if (strcmp(service, "volt_out") == 0)
        pt2Function = volt_out;
    if (strcmp(service, "reboot") == 0)
        pt2Function = reboot;
    if (strcmp(service, "fanon") == 0)
        pt2Function = fan_cmd_on;
    if (strcmp(service, "fanmed") == 0)
        pt2Function = fan_cmd_med;
    if (strcmp(service, "fanmed2") == 0)
        pt2Function = fan_cmd_med2;
    if (strcmp(service, "fanoff") == 0)
        pt2Function = fan_cmd_off;

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
        set_fan_speed(SYS_FAN0, 0);
    return 0;
}

