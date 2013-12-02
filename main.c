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

/* Temporary debug function */
int send_packet1(void)
{
    static uint16_t in_voltage = 0;
    printk("Set voltage %u\n", in_voltage);
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;

    packet1[3] = (in_voltage >> 8);
    packet1[2] = (in_voltage & 0xff);
    in_voltage += 1000;
    if (in_voltage > 50000)
        in_voltage = 0;
    while(bytes_to_send--)
    {
        init_aaps_a(analog_one.hw_ch);
        spi_send_one(&analog_one, ~(packet1[cnter++]));
    }
    return 0;
}

int send_packet3(uint16_t data)
{
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;

    packet3[3] = (data >> 8);
    packet3[2] = (data & 0xff);
    for(int i=0; i < bytes_to_send; i++)
        printk("%u\n", packet3[i]);
    printk("\n");
    while(bytes_to_send--)
    {
        init_aaps_a(analog_zero.hw_ch);
        spi_send_one(&analog_zero, ~(packet3[cnter++]));
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
int get_raw_voltage(void)
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


//  timer1_create_timer(get_raw_voltage, 1150, PERIODIC, 1000);
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
    while(1)
    {
        pending_cmd();
        if (event)
        {
            send_packet3(1234);
            event = 0;
        }
        slave = ipc_which_irq(irq_from_slave);
        if (slave != NO_IRQ) {
            cnt++;
            //printk("irq from slave %u\n", slave);
            if (ipc_transfer_raw(&analog_zero, slave) != IPC_RET_OK)
                goto fatal;
            printk("pkts: %u\n", cnt);
        }
    }
fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
