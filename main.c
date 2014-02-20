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
static uint8_t clind_led_event = 0;
static uint8_t remote_temp_event = 0;
static uint8_t event = 0;
static uint8_t temp_event = 0;
static uint8_t temp_fetch_event = 0;
static uint8_t pdetect_event = 0;
//static struct system_settings sys_settings;

#define CON4  12
#define CON5  6
#define CON6  7
#define CON7  4
#define CON8  5
#define CON9  2
#define CON10 3
#define CON11 0
#define CON12 1
#define CON13 8
#define CON15 9
#define CON16 10
#define CON17 11
#define CON18 13

uint8_t sys_analog = CON11;
uint8_t sys_gui = CON9;

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

#define INPUT_CURRENT false
#define INPUT_VOLTAGE true

/* System control */
uint32_t dac_current_limit = 0;
uint32_t dac_voltage = 0;
uint16_t scale = 1;
bool display_calculated_values = 0;
bool input_calculated_values = 0;
static uint8_t relay_status = 0;
static uint8_t rled_status = 0;
static bool sys_ilimit_active = false;
static bool which_input = INPUT_CURRENT;

void change_scale(void)
{
    switch (scale)
    {
     case 1:
         scale = 10;
     break;
     case 10:
        scale = 100;
     break;
     case 100:
        scale = 500;
     break;
     case 500:
        scale = 1;
     break;
    }
    printk("scale: %u\n", scale);
}
#define MAX_VOLTAGE 0xdb0f //Approximately calibrated max voltage
int trigger_remote_temp_event(void)
{
    remote_temp_event = 1;
    return 0;
}
int activate_clind_led(void)
{
    clind_led_event = 1;
    return 0;
}
int trigger_event(void)
{
    event = 1;
    return 0;
}

int start_temp_event(void)
{
    temp_event = 1;
	return 0;
}
int get_temp_event(void)
{
    temp_fetch_event = 1;
	return 0;
}
int perip_detect_event(void)
{
    static uint8_t cnt = 0;
    pdetect_event = ++cnt;
    return 0;
}

void send_set_led(uint8_t led, uint8_t on)
{
    struct ipc_packet_t pkt =
    {
        .len = 5,
        .cmd = IPC_CMD_SET_LED,
    };
    pkt.data = malloc(2);
    if (pkt.data == NULL)
        printk("malloc0 failed\n");
    pkt.data[0] = led;
    pkt.data[1] = on;

    pkt.crc = crc8(pkt.data, 2);
    if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
        printk("Set led failed\n");
    free(pkt.data);
}
static void send_temp(ow_temp_t *temp, uint8_t sensor)
{
    struct ipc_packet_t pkt =
    {
        .len = 6,
        .cmd = IPC_CMD_DISPLAY_THERMO,
    };
    pkt.data = malloc(3);
    if (pkt.data == NULL)
        printk("malloc0 failed\n");
    pkt.data[0] = sensor;
    pkt.data[1] = temp->temp;
    pkt.data[2] = temp->dec;

    pkt.crc = crc8(pkt.data, 3);
    if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
        printk("Send temp failed\n");
    free(pkt.data);
}

void adc_voltage(uint8_t msb, uint8_t lsb, uint8_t ch, uint64_t *res)
{
    uint16_t value = (msb << 8) | lsb;
    switch(ch)
    {
        case 1:
        *res = 6250000 * (uint64_t)value / 66129;
          break;
        case 2:
            *res = 62500 * (uint64_t)value;
          break;
        case 0:
        case 3:
            *res = 62500 * (uint64_t)value / 1279;
          break;
    }
}

static void send_current(uint8_t msb, uint8_t lsb, uint8_t ch, uint8_t type)
{
    /*
     * Vc=uppmätt värde på ADC ingången
     * där 5A ger Vc=5*40mohm*20=4V
     * Rs=40mohm
     * Gc=20
     * 5.12A max ?
     * 
     * Vc=Iut*Rs*Gc  => Iut=Vc/(Rs*Gc)
     */
    if (ch == 2)
    {
        uint64_t adc; 
        const uint8_t Gc = 20;
        const uint8_t Rs = 40;
        uint32_t Iout = 0;

        adc_voltage(msb, lsb, ch, &adc);
        Iout = adc / (Rs * Gc); /*Add factor 10 to get back to correct base */
        //printk("Iout: %u\n", Iout);
        
        struct ipc_packet_t pkt =
        {
            .len = 8,
            .cmd = IPC_CMD_DISPLAY_CURRENT,
        };
        pkt.data = malloc(4);
        if (pkt.data == NULL)
            printk("send_voltage malloc failed\n");

        /* Voltage in mV */
        pkt.data[0] = type;
        pkt.data[1] = ch;
        pkt.data[2] = Iout >> 8;
        pkt.data[3] = Iout & 0xff;
        pkt.data[4] = Iout >> 16;

        pkt.crc = crc8(pkt.data, 5);
        if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
            printk("send_current failed!\n");
        free(pkt.data);
    }
}

                     /* TODO: Remove type? */
static void send_voltage(uint8_t msb, uint8_t lsb, uint8_t ch, uint8_t type)
{
    uint64_t adc;
    adc_voltage(msb, lsb, ch, &adc);

    struct ipc_packet_t pkt =
    {
        .len = 8,
        .cmd = IPC_CMD_DISPLAY_VOLTAGE,
    };
    pkt.data = malloc(5);
    if (pkt.data == NULL)
        printk("send_voltage malloc failed\n");

    /* Voltage in mV */
    pkt.data[0] = type;
    pkt.data[1] = ch;
    pkt.data[2] = adc & 0xff;
    pkt.data[3] = (adc >> 8) & 0xff;
    pkt.data[4] = adc >> 16;

    pkt.crc = crc8(pkt.data, 5);
    if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
        printk("send_voltage failed!\n");
    free(pkt.data);
}

static void send_dac(uint32_t value, uint8_t type)
{
    struct ipc_packet_t pkt =
    {
        .len = 8,
        .cmd = IPC_CMD_DISPLAY_DAC,
    };
    pkt.data = malloc(5);
    if (pkt.data == NULL)
        printk("dac malloc failed\n");
    pkt.data[0] = type;
    pkt.data[1] = value & 0xff;
    pkt.data[2] = value >> 8;
    pkt.data[3] = value >> 16;;
    pkt.data[4] = value >> 24;

    pkt.crc = crc8(pkt.data, 5);
    if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
        printk("send_dac failed\n");
    free(pkt.data);
}
static void send_adc(uint8_t msb, uint8_t lsb, uint8_t ch, uint8_t type)
{
    struct ipc_packet_t pkt =
    {
        .len = 7,
        .cmd = IPC_CMD_DISPLAY_ADC,
    };
    pkt.data = malloc(4);
    if (pkt.data == NULL)
        printk("malloc1 failed\n");
    pkt.data[0] = type;
    pkt.data[1] = ch;
    pkt.data[2] = msb;
    pkt.data[3] = lsb;

    pkt.crc = crc8(pkt.data, 4);
    if (ipc_put_pkt(sys_gui, &pkt) != IPC_RET_OK)
        printk("send_adc failed\n");
    free(pkt.data);
}
static ipc_ret_t ipc_periph_detect(uint8_t slave)
{
    ipc_ret_t ret = IPC_RET_OK;
    struct ipc_packet_t pkt =
    {
        .len = 4,
        .cmd = IPC_CMD_PERIPH_DETECT,
    };
    pkt.data = malloc(1);
    if (pkt.data == NULL)
        return IPC_RET_ERROR_OUT_OF_MEMORY;

    pkt.data[0] = 0xff; /* Dummy data, not sure if it's needed */

    pkt.crc = crc8(pkt.data, 1);
    ret = ipc_put_pkt(slave, &pkt);
    free(pkt.data);
    return ret;
}
int main(void)
{
    /* This is a temporary configuration of the system */
    //TODO: Channel initialization can't be hard coded like this
    gui.hw_ch = system_channel[sys_gui];
    analog.hw_ch = system_channel[sys_analog];
    /* End temporary configuration of the system */

    ow_temp_t core_temp;
	 #define CS_SDM PJ4	//CS for SD card flash must be low (controls buffer U11)
	 #define CS_FLASH PD6 //CS for flash set low
	 DDRD |= (1<<PD6);
	 DDRJ |= (1<<PJ4);
	 PORTD &= ~(1<<CS_FLASH);
	 PORTJ &= ~(1<<CS_SDM);

    /* Enable external SRAM early */
    XMCRA |= (1<<SRE);

    rst_save_reason();
    led_init();
    wdt_enable(WDTO_8S);
	
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

mem_test();

    printk("Found %u sensors\n", ow_num_sensors);
    //temp.temp = 0;
    //temp.dec = 0;


  timer1_create_timer(trigger_remote_temp_event, 750, PERIODIC, 500);
  timer1_create_timer(trigger_event, 100, PERIODIC, 0);
  timer1_create_timer(activate_clind_led, 150, PERIODIC, 20);
  timer1_create_timer(start_temp_event, 1000, PERIODIC, 0);
  timer1_create_timer(get_temp_event, 1000, PERIODIC, 200);
//  timer1_create_timer(card_detect, 500, PERIODIC, 0);
//  timer1_create_timer(send_packet1, 100, PERIODIC, 5100);
//  timer1_create_timer(send_packet2, 2000, ONE_SHOT, 0 );
//  timer1_create_timer(send_packet1, 2000, ONE_SHOT, 1000);

    /* Detect peripherals */
    timer1_create_timer(perip_detect_event, 500, ONE_SHOT, 0);

    uint16_t cnt = 0;
    uint8_t slave = NO_IRQ;
    static uint8_t temp_sensors_id = 0;
	static uint8_t num_aaps_a_sensors = 0;
    init_aaps_a(gui.hw_ch);
    init_aaps_a(analog.hw_ch);

    struct ipc_packet_t pkt;
    
    while(1)
    {
        wdt_reset();
		pending_cmd();
        if (pdetect_event)
        {
            ipc_ret_t ret = ipc_periph_detect(pdetect_event-1);
            if (ret != IPC_RET_OK)
                ;//printk("CH%u not connected.\n", pdetect_event-1);
            if (pdetect_event <= HW_NBR_OF_CHANNELS)
                timer1_create_timer(perip_detect_event, 10, ONE_SHOT, 0);

            pdetect_event = 0;
        }
        if (temp_fetch_event)
        {
            if (get_temp(&core_temp) == OW_RET_OK)
            {
               
				send_temp(&core_temp, 5);
            }
            temp_fetch_event = 0;
        }
        if (temp_event)
        {
            trigger_conv_t();
            temp_event = 0;
        }
        if (remote_temp_event)
        {
            
           get_aaps_a_temp(temp_sensors_id, sys_analog);
		   temp_sensors_id++;
           temp_sensors_id %= num_aaps_a_sensors;
		  	   
		   remote_temp_event = 0;
        }
        if (event)
        {
            /* Handle IRQ events */
            static uint8_t ch = 0;
            get_adc(ch++ % 8, sys_analog);
            event = 0;
        }
        if (clind_led_event && sys_ilimit_active)
        {
            rled_status ^= 1;
            send_set_led(IPC_LED_RED, rled_status);
            clind_led_event = 0;
        }

        slave = ipc_which_irq(irq_from_slave);
        if (slave != NO_IRQ) {
            cnt++;
            //printk("irq from slave %u\n", slave);
            ipc_ret_t result = ipc_get_pkt(slave, &pkt);
            if (result == IPC_RET_OK)
            {
                if (crc8(pkt.data, pkt.len - IPC_PKT_OVERHEAD) == pkt.crc)
                {   ow_temp_t t;
                    switch (pkt.cmd)
                    {
                        case IPC_DATA_PERIPH_DETECT:
                            printk("CH%u detected\n", slave);
                            printk("   %u sensors\n", pkt.data[2]);
							if (pkt.data[2] != 0)
							{
								num_aaps_a_sensors = pkt.data[2];
							}
							
                            break;
                        case IPC_DATA_THERMO:
							
                            t.temp = pkt.data[1]; t.dec = pkt.data[2];
                            send_temp(&t, pkt.data[0]);
                            break;
                        case IPC_DATA_CURRENT:
                            send_adc(pkt.data[2],  pkt.data[3],
                                     pkt.data[1], pkt.data[0]);
                            send_current(pkt.data[2],  pkt.data[3],
                                     pkt.data[1], pkt.data[0]);
                            send_dac(dac_current_limit, pkt.data[0]);
                            break;
                        case IPC_DATA_VOLTAGE:
                            send_adc(pkt.data[2],  pkt.data[3],
                                     pkt.data[1], pkt.data[0]);
                            send_voltage(pkt.data[2],  pkt.data[3],
                                     pkt.data[1], pkt.data[0]);
                            send_dac(dac_voltage, pkt.data[0]);
                            break;
                        case IPC_DATA_ENC_BTN:
                            change_scale();
                            break;
                        case IPC_DATA_ENC_DB_BTN:
                            which_input = which_input ? INPUT_CURRENT :
							INPUT_VOLTAGE;
                            printk("Changed input to %s\n",
                                which_input ? "voltage" : "current");
                            break;
                        case IPC_DATA_ENC_LONGPRESS:
                            relay_status ^= 1;
                            set_relay(relay_status, sys_analog);
                            break;
                        case IPC_DATA_ENC_CW:
                            if (which_input)
                            {
                                if ((uint16_t)dac_voltage + scale >
                                    dac_voltage)
                                {
                                    voltage(dac_voltage, sys_analog);
                                    dac_voltage += scale;
                                    printk("rot+ %lu\n", dac_voltage);
                                }
                            }
                            else
                            {
                                if ((uint16_t)dac_current_limit + scale >
                                    dac_current_limit)
                                {
                                    current(dac_current_limit, sys_analog);
                                    dac_current_limit += scale;
                                    printk("rot+ %lu\n", dac_current_limit);
                                }
                            }
                            break;
                        case IPC_DATA_ENC_CCW:
                            if (which_input)
                            {
                                if (dac_voltage - scale < dac_voltage)
                                {
                                        voltage(dac_voltage, sys_analog);
                                        dac_voltage -= scale;
                                        printk("rot- %lu\n", dac_voltage);
                                } else printk("bottom\n");
                            }
                            else
                            {
                                if (dac_current_limit - scale < dac_current_limit)
                                {
                                    current(dac_current_limit, sys_analog);
                                    dac_current_limit -= scale;
                                    printk("rot- %lu\n", dac_current_limit);
                                } else printk("top\n");
                            }
                            break;
                        case IPC_DATA_ENC_SW0:
                            //display_calculated_values =
                            //    display_calculated_values ? false : true;
                            //printk("Display calculated: %u\n",
                            //       display_calculated_values);
                            printk("Changed display page\n");
                            break;
                        case IPC_DATA_ENC_SW1:
                            dac_current_limit = 0;
                            current(dac_current_limit, sys_analog);
                            break;
                        case IPC_DATA_ENC_SW2:
                            break;
                        case IPC_DATA_CLIND:
                                sys_ilimit_active = pkt.data[0] ? true : false;
                                if (sys_ilimit_active)
                                {
                                    if (relay_status)
                                        send_set_led(IPC_LED_GREEN, !pkt.data[0]);
                                    else
                                        send_set_led(IPC_LED_RED, 0);
                                }
                                else
                                {
                                    if (relay_status)
                                    {
                                        send_set_led(IPC_LED_GREEN, 1);
                                        send_set_led(IPC_LED_RED, 0);
                                    }
                                    else
                                        send_set_led(IPC_LED_RED, 0);

                                }
                            printk("CLIND: %u\n", sys_ilimit_active);
                            break;
                        default:
                            printk("Unknown data type: 0x%x\n", pkt.cmd);
                    }
                    //printk("len: %u\n", pkt.len);
                    //printk("cmd: 0x%02x\n", pkt.cmd);
                    //printk("crc: 0x%02x\n", pkt.crc);
                    //for (uint8_t i = 0; i < pkt.len - IPC_PKT_OVERHEAD; i++)
                    //    printk("d%02u: 0x%x\n", i, pkt.data[i]);
                    //printk("pkts: %u\n", cnt);
                }
                else
                  printk("CRC failed from slave %u", slave);

                free(pkt.data);
                pkt.data = NULL;
            }
            else
            {
                printk("get failed err:%u\n", result);
            }
        }
    }
//fatal:
    printk("Fatal error!\n");
    while(1);

    return 0;
}
