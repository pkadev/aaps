#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "led.h"
#include "memtest.h"
#include "m128_hal.h"
#include "uart.h"
#include "timer.h"
#include "timer1.h"
#include "cmd.h"
#include "1wire.h"
#include <avr/wdt.h>
#include "fan.h"
#include "ds3234.h"
#include "settings.h"
#include "ipc.h"
#include "hw_channel.h"
#include "reset.h"
#include "list.h"
#include "storage.h"
#include "spi.h"
#include "aaps_a.h"

//static struct system_settings sys_settings;

int enable_led1(void)
{
    led_ctrl(LED1, LED_ON);
    return 0;
}

int disable_led1(void)
{
    led_ctrl(LED1, LED_OFF);
    return 0;
}

int enable_led0(void)
{
    led_ctrl(LED0, LED_ON);
    return 0;
}

int disable_led0(void)
{
    led_ctrl(LED0, LED_OFF);
    return 0;
}

struct spi_device_t analog_zero =
{
    //.opto_coupled = true,
    //.hw_ch = system_channel[0],
    .init = init_aaps_a,
};
struct spi_device_t analog_one =
{
    //.opto_coupled = true,
    //.hw_ch = system_channel[0],
    .init = init_aaps_a,
};

/* TODO: This function must be made more dynamic 
 * and moved to proper source file.
 * Lookup of spi channels can't be hard coded.
 */
struct spi_device_t *channel_lookup(uint8_t ch)
{
    switch(ch)
    {
        case 0: { return &analog_zero; break; }
        case 1: { return &analog_one; break; }
        default:
            printk("Error. No such channel [%u]\n", ch);
    }
    return NULL;
}
uint8_t packet0[] =
{
    0x01, 0x02, 0xDA, 0x7A, 0x42,
};
#define MAX_VOLTAGE 0xdb0f //Approximately calibrated max voltage
uint8_t packet1[] =
{
    IPC_CMD_SET_VOLTAGE,
    0x02,
    0x59,
    0x16,
    0xEF,
};

uint8_t packet2[] =
{
    IPC_CMD_SET_CURRENT_LIMIT,
    0x02,
    0xff,
    0xf4,
    0xbc,
};

/* Temporary debug function */
int send_packet1(void)
{
    printk("Set voltage\n");
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;

    while(bytes_to_send--)
    {
        init_aaps_a(analog_zero.hw_ch);
        spi_send_one(&analog_zero, ~(packet1[cnter++]));
    }
    return 0;
}

/* Temporary debug function */
int send_packet2(void)
{
    printk("Set current\n");
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    while(bytes_to_send--)
    {
        init_aaps_a(analog_zero.hw_ch);
        spi_send_one(&analog_zero, ~(packet2[cnter++]));
    }
    return 0;
}

int main(void)
{
    /* Enable external SRAM early */
    XMCRA |= (1<<SRE);

    rst_save_reason();
    led_init();
    //wdt_enable(WDTO_4S);
    hw_init();

    //TODO: Channel initialization can't be hard coded like this
    analog_zero.hw_ch = system_channel[0];
    analog_one.hw_ch = system_channel[1];

    uint8_t ow_num_sensors = ow_num_devices();
    ow_devices = malloc(sizeof(ow_device_t)*ow_num_sensors);
    ow_get_devices(ow_devices);
    //ow_convert_temp_async(&(ow_devices[0]));
    DDRE |= (1<<PE2);
    fan_init();

    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);

    uart_init();
    rst_print_reason();
    cmd_init();
    timer_init();
    ds3234_init();
    timer1_init();

    ow_print_device_addr(&(ow_devices[0]));
    /* PCINT10 from CH12 */
    //PCMSK1 |= (1<< PCINT10);
    ///* PCINT11 from CH13 */
    //PCMSK1 |= (1<< PCINT11);
    //PCICR |= (1<<PCIE1);

//mem_test();

    printk("Found %u sensors\n", ow_num_sensors);
    //temp.temp = 0;
    //temp.dec = 0;


    timer1_create_timer(trigger_conv_t, 10000, PERIODIC, 0);
    timer1_create_timer(get_temp, 10000, PERIODIC, 200);
    //timer1_create_timer(card_detect, 500, PERIODIC, 0);
//    timer1_create_timer(send_packet1, 210, PERIODIC, 2100);
    timer1_create_timer(send_packet2, 2000, ONE_SHOT, 0 );
    timer1_create_timer(send_packet1, 2000, ONE_SHOT, 1000);
//    timer1_create_timer(send_packet1, 1000, ONE_SHOT, 6000);
//    timer1_create_timer(send_packet1, 1000, ONE_SHOT, 4000);

    /* Detect peripherals */
//    uint8_t periph_type;
//    for (uint8_t i = 0; i < HW_NBR_OF_CHANNELS; i++) {
//        if (ipc_periph_detect(&analog_zero, &periph_type) != IPC_RET_OK)
//        printk("Error detecting peripherals\n");
  //  }

    uint8_t cnt = 0;

    while(1)
    {
        cnt++;
        pending_cmd();
        if (irq_from_slave != NO_IRQ) {
            //printk("irq_from_slave is %u\n", irq_from_slave);
            //Find out why slave is bothering us
            ipc_irq_reason_t rsn;
             /* Do the lookup up of channels properly */

            if (ipc_get_irq_reason(channel_lookup(irq_from_slave), &rsn) == IPC_RET_OK)
            {
                if (rsn == IPC_CMD_DATA_AVAILABLE)
                {
                    uint8_t *buf;
                    uint8_t len;

                    if (ipc_get_data_len(channel_lookup(irq_from_slave), &len) == IPC_RET_OK)
                        ;
                    else
                        printk("get len failed\n");

                    buf = malloc(len + 1);

                    if (buf == NULL)
                        printk("Malloc failed\n");

                    if (ipc_get_available_data(channel_lookup(irq_from_slave), buf, len) == IPC_RET_OK) {
                        buf[len] = '\0';
                    } else
                        printk("get data failed\n");

                    /*                                           *
                     * TODO: Move this section to a separate     *
                     * function that formats data based on data  *
                     * types.                                    *
                     *                                           */

                    if (*buf == IPC_DATA_THERMO)
                    {
                        printk("Received a temperature\n");
                        ow_temp_t  a_temp;
                        a_temp.temp = buf[1];
                        a_temp.dec = buf[2];
                        printk("Type - temp: %u.%u°C\n", a_temp.temp, a_temp.dec);
                    }
                    else if (*buf == IPC_DATA_VOLTAGE)
                    {
                        printk("Received a voltage reading\n");
                        uint16_t voltage = (buf[1] << 8) | (buf[2] & 0xFF);
                        printk("Voltage: %u\n", voltage);
                    }
                    else if (*buf == IPC_DATA_CURRENT)
                    {
                        printk("Received a current reading\n");
                        uint16_t current = (buf[1] << 8) | (buf[2] & 0xFF);
                        printk("Current: %u\n", current);
                    }
                    else if(*buf == IPC_DATA_ASCII)
                    {
                        //printk("Received ascii string\n");
                        printk((char *)(buf+1));
                    }
                    else
                        printk("Unknown data type received 0x%x\n", *buf);
                    free(buf);
                }
            }
            irq_from_slave = NO_IRQ;
        }
    }
//fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
