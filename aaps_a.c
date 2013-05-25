#include <avr/io.h>
#include <stdio.h>
#include "m128_hal.h"
#include "aaps_a.h"
#include "spi.h"
#include "uart.h"

#define CS_PIN 3
#define CS_DDR DDRL
#define CS_PORT PORTL
#define CS_LOW() (CS_PORT &= ~(1<<CS_PIN))
#define CS_HIGH() (CS_PORT |= (1<<CS_PIN))

void init_aaps_a(void)
{
    /* Set MOSI, CS and SCK output */
    DDRB |= (1<<MOSI) | (1<<SCK);
    CS_DDR |= (1<<CS_PIN);
    DDRB |= (1<<PB0);

    /*
     * Enable SPI, Master, set clock rate.
     */
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<SPR1);
    //SPSR = (1<<SPI2X);
    disable_aaps_a();
}

void enable_aaps_a(void)
{
    printk("enable_aaps_a\n");
    CS_HIGH();
}

void disable_aaps_a(void)
{
    CS_LOW();
}

//uint8_t aaps_a_transfer(uint8_t *buf, size_t len)
//{
//    init_aaps_a();
//    uint8_t rx;
//    enable_aaps_a();
//    rx = spi_transfer(~(*buf));
//    disable_aaps_a();
//    return rx;
//}
