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

static uint8_t event = 0;
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


/* TODO: This function must be made more dynamic 
 * and moved to proper source file.
 * Lookup of spi channels can't be hard coded.
 */
uint8_t packet0[] =
{
    0x01, 0x02, 0xDA, 0x7A, 0x42,
};
#define MAX_VOLTAGE 0xdb0f //Approximately calibrated max voltage
uint8_t packet1[] =
{
    IPC_CMD_SET_VOLTAGE,
    0x02,
    0xf9,
    0x16,
    0xEF,
};
uint8_t packet3[] =
{
    IPC_CMD_PUT_DATA,
    0x02,
    0xf9,
    0x16,
    0xCC,
};

uint8_t packet2[] =
{
    IPC_CMD_SET_CURRENT_LIMIT,
    0x02,
    0xff,
    0xf4,
    0xbc,
};

int trigger_event(void)
{
    //get_adc(0, channel_lookup(1));
    event = 1;
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
    init_aaps_a(analog_zero.hw_ch);
    init_aaps_a(analog_one.hw_ch);

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

    printk("DS3234 ctrl reg: 0x%x\n", ds3234_read_ctrl_reg());
    ow_print_device_addr(&(ow_devices[0]));

//mem_test();

    printk("Found %u sensors\n", ow_num_sensors);
    //temp.temp = 0;
    //temp.dec = 0;


  timer1_create_timer(trigger_event, 100, PERIODIC, 0);
//  timer1_create_timer(trigger_conv_t, 10000, PERIODIC, 0);
//  timer1_create_timer(get_temp, 10000, PERIODIC, 200);
//  timer1_create_timer(card_detect, 500, PERIODIC, 0);
//  timer1_create_timer(send_packet1, 100, PERIODIC, 5100);
//  timer1_create_timer(send_packet2, 2000, ONE_SHOT, 0 );
//  timer1_create_timer(send_packet1, 2000, ONE_SHOT, 1000);

    /* Detect peripherals */
//    uint8_t periph_type;
//    for (uint8_t i = 0; i < HW_NBR_OF_CHANNELS; i++) {
//        if (ipc_periph_detect(&analog_zero, &periph_type) != IPC_RET_OK)
//        printk("Error detecting peripherals\n");
  //  }

    uint16_t cnt = 0;
    uint8_t slave = NO_IRQ;
    init_aaps_a(analog_zero.hw_ch);
    struct ipc_packet_t pkt;
    while(1)
    {
        pending_cmd();
        if (event)
        {
            /* Handle IRQ events */
            struct ipc_packet_t pkt =
            {
                .len = 8,
                .cmd = IPC_CMD_PUT_DATA,
                .crc = 0x11,
                //.data = { 'r', '\0' },
            };
            pkt.data = malloc(4);
            if (pkt.data == NULL)
                printk("malloc failed\n");
            pkt.data[0] = 'p';
            pkt.data[1] = 'e';
            pkt.data[2] = 'r';
            pkt.data[3] = '!';
            pkt.data[4] = '\0';

            if (ipc_put_pkt(0, &pkt) != IPC_RET_OK)
                printk("put packet failed\n");
            event = 0;
            free(pkt.data);
        }
        slave = ipc_which_irq(irq_from_slave);
        if (slave != NO_IRQ) {
            cnt++;
            //printk("irq from slave %u\n", slave);
            if (ipc_get_pkt(slave, &pkt) == IPC_RET_OK)
            {
                printk("len: %u\n", pkt.len);
                printk("cmd: %u\n", pkt.cmd);
                printk("crc: %u\n", pkt.crc);
                for (uint8_t i = 0; i < pkt.len - IPC_PKT_OVERHEAD; i++)
                    printk("d%02u: 0x%x\n", i, pkt.data[i]);
                printk("pkts: %u\n", cnt);
            }
            else
            {
                printk("get failed\n");
            }
        }
    }
//fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
