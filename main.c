#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include "m128_hal.h"
#include "uart.h"
#include "timer.h"
#include "cmd.h"
#include "1wire.h"
#include <avr/wdt.h>
#include "fan.h"

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


int main(void)
{
    fan_init();
//    set_fan_speed(SYS_FAN0, 250);
    //set_fan_speed(SYS_FAN1, 200);

    uint8_t test = 1;
    uint8_t test1= 1;
    uint8_t test2= 1;
    //XMCRA |= (1<<SRE);
    //XMCRA |= (1<<SRW01) | (1<<SRW11);

    wdt_enable(WDTO_8S);
    
    STATUS_REGISTER |= (1<<STATUS_REGISTER_IT);
    uart_init();
    cmd_init();
    timer_init();
    printk("stack: 0x%x\n", &test);
    printk("stack: 0x%x\n", &test1);
    printk("stack: 0x%x\n", &test2);
    uint8_t ow_num_sensors = ow_num_devices();
    ow_device_t *ow_devices;

    /* Memtest pattern fill */
//    int j;
//for (j=0; j<20; j++)
//{
//    uint16_t size = 2000;
//    uint8_t *p_data = (uint8_t *)malloc(size);
//    if (p_data != NULL)
//    {
//        printk("Memtest starts at: 0x%x\n", p_data);
//        memset(p_data, 0xDA, size);
//        uint8_t res = cmp_pattern(p_data, 0xDA, size-1);    
//        if (res != 0)
//            printk("Pattern match failed!\n");
//    } else {
//        goto fatal;
//    }
//
//    free(p_data); 
//    p_data = NULL;
//}
   //memtest(); 
    if (ow_num_sensors) {
        printk("Detected %u 1-Wire devices\n", ow_num_sensors);
        ow_devices = malloc(sizeof(ow_device_t)*ow_num_sensors);
        ow_get_devices(ow_devices);
    } else {
        goto fatal;
    }

    printk("Setting: 0x%x\n", eeprom_read_word(0));

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
    //int fan_speed = 0;
    while(1)
    {
        _delay_ms(250);
        pending_cmd();
        wdt_reset();
        
    }
fatal:
    printk("Fatal error!\n");
    while(1);
    return 0;
}

