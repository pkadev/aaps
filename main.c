#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
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

    ow_device_t *ow_devices;
uint8_t cmp_pattern(uint8_t *p, uint8_t pattern, size_t size)
{
    uint8_t *p_end = (p + size);
    while(p < p_end) {
        if(*p != pattern) {
            printk("Pattern match failed at 0x%x [%x]\n", *p);
            return -1;
        }
        p++;
    }
    return 0;
}

volatile uint8_t irq_from_slave = 0;

ISR(INT5_vect) //IRQ from aaps_a
{
    //printk("irq - periph!\n");
    irq_from_slave = 1;
}

//static struct system_settings sys_settings;

int test_cb1(void)
{
    printk("1\n");
    return 0;
}
int test_cb2(void)
{
    ow_temp_t temp;
    if (get_scratch_pad_async(&(ow_devices[0]), &temp) == OW_RET_OK)
            printk("Ambient temperature: %u.%u°C\n",temp.temp, temp.dec);
    ow_convert_temp_async(&(ow_devices[0]));
    return 0;
}
int main(void)
{
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

    //XMCRA |= (1<<SRE);
    //XMCRA |= (1<<SRW01) | (1<<SRW11);

    struct rtc_time time;
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);
    //_delay_ms(1000);
    uart_init();
    rst_print_reason();
    cmd_init();
    timer_init();
    ds3234_init();
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

    printk("reg: 0x%x\n", ctrl_reg);
    ds3234_write_reg(0x8e, 0x00);
    printk("reg: 0x%x\n", ds3234_read_ctrl_reg());
    ret = ipc_periph_detect(&m48, &periph_type);

    if (ret != IPC_RET_OK) {
        printk("Error: 0x%x\n", ret);
    } else {
        printk("Detected peripheral. Type: %u\n", periph_type);
    }

    if (ow_num_sensors) {
        printk("Detected %u 1-Wire devices\n", ow_num_sensors);
        ow_devices = malloc(sizeof(ow_device_t)*ow_num_sensors);
        ow_get_devices(ow_devices);
    } else {
        goto fatal;
    }

    //for(uint8_t i=0; i<ow_num_sensors; i++)
    //    ow_print_device_addr(ow_devices);

    ow_temp_t temp;
    //for (uint8_t i=0; i<ow_num_sensors; i++)
    //{
    //    if (ow_read_temperature(&(ow_devices[i]), &temp)) {
    //        printk("Ambient temperature: %u.%u°C\n",temp.temp, temp.dec);
    //    } else {
    //        printk("CRC failed\n");
    //    }
    //}
    timer1_init();
    temp.temp = 0;
    temp.dec = 0;
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 95);
    ow_convert_temp_async(&(ow_devices[0]));
    timer1_create_timer(test_cb2, 1750, PERIODIC);

    //timer1_create_timer(test_cb2, 7000);
    //timer1_create_timer(test_cb2, 7000);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb1, 1);
    //timer1_create_timer(test_cb2, 5); //Should not work since timer is already registered
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
