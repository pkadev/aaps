#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "uart.h"
#include "ipc.h"
#include "ds3234.h"

void ds3234_init(void)
{
    /* Set MOSI and SCK output */
    DDR_SPI |= (1<<MOSI) | (1<<SCK);
    /* CS is output */
    RTC_CS_DDR |= (1<<RTC_CS_PIN);
    /* SPI SS pin as output to aviod slave mode */
    DDRB |= (1<<PB0);

    /*
     * Enable SPI, Master, set clock rate.
     * ds3234 can run in 4MHz max
     */
    SPCR = (1<<SPE) /*| (1<<SPIE)*/ | (1<<MSTR) | (1<<SPR0) | (1<<CPHA);
    SPSR = (1<<SPI2X);
    CS_HIGH();
}


void ds3234_write_reg(uint8_t reg, uint8_t val)
{
    CS_LOW();
    spi_transfer(reg);
    spi_transfer(val);
    CS_HIGH();
}

uint8_t ds3234_read_ctrl_reg()
{
    uint8_t val = 0;

    CS_LOW();
    spi_transfer(0x0e);
    val = spi_transfer(DS3234_DUMMY_BYTE);
    CS_HIGH();
    return val;
}

void ds3234_get_time(struct rtc_time *time)
{
    uint8_t reg;

    CS_LOW();
    spi_transfer(0x00);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->sec = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    CS_LOW();
    spi_transfer(0x01);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->min = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    CS_LOW();
    spi_transfer(0x02);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->hour = (((reg & 0x10) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    CS_LOW();
    spi_transfer(0x04);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->date = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    CS_LOW();
    spi_transfer(0x05);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->month = (((reg & 0x10) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    CS_LOW();
    spi_transfer(0x06);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->year = (((reg & 0x10) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();

    //CS_LOW();
    //spi_transfer(0x81);
    //reg = spi_transfer(0x55);
    //CS_HIGH();
}
