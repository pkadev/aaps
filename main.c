#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/interrupt.h>
#include "m128_hal.h"
#include "uart.h"
#include "timer.h"
#include "cmd.h"
#include "1wire.h"
#include <avr/wdt.h>
#include "fan.h"
#include "ds3234.h"
#include "settings.h"

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

//static struct system_settings sys_settings;

int main(void)
{
    fan_init();
    //XMCRA |= (1<<SRE);
    //XMCRA |= (1<<SRW01) | (1<<SRW11);

    //struct rtc_time time;
    wdt_enable(WDTO_8S);
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);
    uart_init();
    cmd_init();
    timer_init();
    ds3234_init();
   _delay_ms(500); 
    uint8_t ctrl_reg = ds3234_read_ctrl_reg();
    printk("reg: 0x%x\n", ctrl_reg);
    ds3234_write_reg(0x08);
    //uint8_t ow_num_sensors = ow_num_devices();
    //ow_device_t *ow_devices;
    //settings_get_settings(&sys_settings);
    
    ctrl_reg = ds3234_read_ctrl_reg();
    printk("reg: 0x%x\n", ctrl_reg);
    ds3234_write_reg(0x98);
 
    //if (ow_num_sensors) {
    //    printk("Detected %u 1-Wire devices\n", ow_num_sensors);
    //    ow_devices = malloc(sizeof(ow_device_t)*ow_num_sensors);
    //    ow_get_devices(ow_devices);
    //} else {
    //    goto fatal;
    //}


 //   for(uint8_t i=0; i<ow_num_sensors; i++)
 //       ow_print_device_addr(ow_devices);
    
 //   ow_temp_t temp; 
 //   for (uint8_t i=0; i<ow_num_sensors; i++)
 //   {
 //       if (ow_read_temperature(&(ow_devices[i]), &temp)) {
 //           printk("Ambient temperature: %u.%u°C\n",temp.temp, temp.dec);
 //       } else {
 //           printk("CRC failed\n");
 //       }
 //   }
    //int fan_speed = 0;
    while(1)
    {
    //    _delay_ms(250);
        pending_cmd();
        wdt_reset();
    ctrl_reg = ds3234_read_ctrl_reg();
    printk("reg: 0x%x\n", ctrl_reg);
    _delay_ms(400);
    }
//fatal:
    printk("Fatal error!\n");
    while(1);
    return 0;
}

