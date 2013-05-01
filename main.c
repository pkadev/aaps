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


volatile uint8_t irq_from_slave = 0;

ISR(INT5_vect) //IRQ from aaps_a
{
    //printk("irq - periph!\n");
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


int main(void)
{
    /* Enable external SRAM early */
    XMCRA |= (1<<SRE);

    rst_save_reason();
    struct spi_device_t m48;
    m48.cs_pin = 2;
    uint8_t periph_type = 0;
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;
    wdt_enable(WDTO_4S);
    hw_init();
    m48.hw_ch = &(system_channels[0]);

    DDRE |= (1<<PE2);
    fan_init();

    led_init();
    struct rtc_time time;
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);
    //_delay_ms(1000);
    uart_init();
    rst_print_reason();
    cmd_init();
    timer_init();
    ds3234_init();
    struct list_node_t *list_head = list_init();
    if (list_head == NULL)
        printk("Failed to init list\n");
mem_test();
    uint8_t ctrl_reg = ds3234_read_ctrl_reg();

    ds3234_get_time(&time);

    printk("%02u%02u%02u %02u:%02u:%02u\n", time.year,
                                            time.month,
                                            time.date,
                                            time.hour,
                                            time.min,
                                            time.sec);

    /* Configure IRQ pin from 'futur' periph */
    EICRB |= (1<<ISC51) | (1<<ISC50);
    EIMSK |= (1<<INT5);
    /* End Configure IRQ */
    uint8_t ow_num_sensors = ow_num_devices();
    //settings_get_settings(&sys_settings);

#define DBG(x) x
    printk("reg: 0x%x\n", ctrl_reg);
    ds3234_write_reg(0x8e, 0x00);
    DBG(printk("reg: 0x%x\n", ds3234_read_ctrl_reg());)
    ret = ipc_periph_detect(&m48, &periph_type);
    if (ret != IPC_RET_OK) {
        printk("Error: 0x%x\n", ret);
    } else {
        printk("Detected peripheral. Type: %u\n", periph_type);
    }

    if (ow_num_sensors || 1) {
        printk("Detected %u 1-Wire devices\n", ow_num_sensors);
        ow_devices = malloc(sizeof(ow_device_t)*ow_num_sensors);
        ow_get_devices(ow_devices);
    } else {
        goto fatal;
    }

    for(uint8_t i=0; i<ow_num_sensors; i++)
        ow_print_device_addr(ow_devices);

    ow_temp_t temp;
    for (uint8_t i=0; i<ow_num_sensors; i++)
    {
        if (ow_read_temperature(&(ow_devices[i]), &temp)) {
            printk("Ambient temperature: %u.%u°C\n",temp.temp, temp.dec);
        } else {
            printk("CRC failed\n");
        }
    }

    timer1_init();
    temp.temp = 0;
    temp.dec = 0;

    ow_convert_temp_async(&(ow_devices[0]));
    timer1_create_timer(trigger_conv_t, 5000, PERIODIC, 0);
    timer1_create_timer(get_temp, 5000, PERIODIC, 200);
    timer1_create_timer(enable_led0, 2500, PERIODIC, 0);
    timer1_create_timer(disable_led0, 2500, PERIODIC, 10);
    timer1_create_timer(card_detect, 500, PERIODIC, 0);
    //timer1_create_timer(test_cb2, 5, ONE_SHOT, 0);

    timer1_create_timer(enable_led1, 1500, PERIODIC, 0);
    timer1_create_timer(disable_led1, 1500, PERIODIC, 100);


    //int fan_speed = 0;
    uint8_t cnt = 0;
    while(1)
    {
        printk("Entering main loop\n");
        while(1) {
            pending_cmd();
            wdt_reset();
        }
    //ctrl_reg = ds3234_read_ctrl_reg();
    //printk("reg: 0x%x\n", ctrl_reg);
        //ipc_send(&m48, cnt);
        cnt++;

        if (irq_from_slave) {
            //Find out why slave is bothering us
            ipc_irq_reason_t rsn;
            if (ipc_get_irq_reason(&m48, &rsn) == IPC_RET_OK)
            {
                if (rsn == 0x10)
                {
                    char *str;
                    uint8_t len;

                    if (ipc_get_data_len(&m48, &len) == IPC_RET_OK)
                        ;
                    else
                        printk("get len failed\n");

                    str = malloc(len + 1);

                    if (str == NULL)
                        printk("Malloc failed\n");

                    if (ipc_get_available_data(&m48, str, len) == IPC_RET_OK) {
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
fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
