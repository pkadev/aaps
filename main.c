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

volatile uint8_t irq_from_slave = 0;

ISR(INT5_vect) //IRQ from aaps_a
{
    printk("irq - periph!\n");
    irq_from_slave = 1;
}

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
#define IRQ_CH12 PCINT1_vect

ISR(IRQ_CH12)
{
    if (!(PINJ & (1<<PJ1))) {
        irq_from_slave = 1;
        //printk("IRQ from CH12\n");
    }
}

/*
 * Static functions for aaps_a that
 * should move to HW dependant file
 */
/* Below defines should be made global
 * or at least available generically for the ones that need it
 */
/* Above defines should be made global
 * or at least available generically for the ones that need it
 */
/* END HW dependant */


struct spi_device_t aaps_a =
{
    .init = init_aaps_a,
    .enable = enable_aaps_a,
    .disable = disable_aaps_a,
    .transfer = aaps_a_transfer,
};

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
static int packet_counter = 0;
int send_packet0(void)
{
    printk("Packe number: %u\n", packet_counter++);
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    while(bytes_to_send--)
    {
        aaps_a_transfer(&(packet0[cnter++]), 1);
    }
    return 0;
}
int send_packet1(void)
{
    printk("Set voltage\n");
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    while(bytes_to_send--)
    {
        aaps_a_transfer(&(packet1[cnter++]), 1);
    }
    return 0;
}
int send_packet2(void)
{
    printk("Set current\n");
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    while(bytes_to_send--)
    {
        aaps_a_transfer(&(packet2[cnter++]), 1);
    }
    return 0;
}
int main(void)
{
    /* Enable external SRAM early */
    XMCRA |= (1<<SRE);

    DDRL |= (1<<PL3);

    rst_save_reason();
    led_init();
    //wdt_enable(WDTO_4S);
    hw_init();

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
    PCMSK1 |= (1<< PCINT10);
    PCICR |= (1<<PCIE1);

//mem_test();

    /* Configure IRQ pin from 'futur' periph */
    EICRB |= (1<<ISC51) | (1<<ISC50);
    EIMSK |= (1<<INT5);
    /* End Configure IRQ */
    printk("Found %u sensors\n", ow_num_sensors);
    //temp.temp = 0;
    //temp.dec = 0;


    timer1_create_timer(trigger_conv_t, 10000, PERIODIC, 0);
    timer1_create_timer(get_temp, 10000, PERIODIC, 200);
    //timer1_create_timer(card_detect, 500, PERIODIC, 0);
//    timer1_create_timer(send_packet0, 150, PERIODIC, 2000);
//    timer1_create_timer(send_packet1, 210, PERIODIC, 2100);
    timer1_create_timer(send_packet2, 2000, ONE_SHOT, 0 );
    timer1_create_timer(send_packet1, 2000, ONE_SHOT, 1000);
//    timer1_create_timer(send_packet1, 1000, ONE_SHOT, 6000);
//    timer1_create_timer(send_packet0, 1000, ONE_SHOT, 5000);
//    timer1_create_timer(send_packet1, 1000, ONE_SHOT, 4000);
//    timer1_create_timer(send_packet2, 1000, ONE_SHOT, 3000);
//    timer1_create_timer(send_packet1, 1000, ONE_SHOT, 2000);


    //int fan_speed = 0;
    uint8_t cnt = 0;

    while(1)
    {
        cnt++;
        pending_cmd();
        if (irq_from_slave) {
            //Find out why slave is bothering us
            ipc_irq_reason_t rsn;
            if (ipc_get_irq_reason(&aaps_a, &rsn) == IPC_RET_OK)
            {
                if (rsn == IPC_CMD_DATA_AVAILABLE)
                {
                    char *str;
                    uint8_t len;

                    if (ipc_get_data_len(&aaps_a, &len) == IPC_RET_OK)
                        ;
                    else
                        printk("get len failed\n");

                    str = malloc(len + 1);

                    if (str == NULL)
                        printk("Malloc failed\n");

                    if (ipc_get_available_data(&aaps_a, str, len) == IPC_RET_OK) {
                        str[len] = '\0';
                    }
                    else
                        printk("get data failed\n");

                    printk(str);
                    free(str);
                }
            }
            irq_from_slave = 0;
        }
    }
//fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
