#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "uart.h"
#include "ipc.h"
#include "ds3234.h"

/** hardware specific */
#define DDR_SPI                                 (DDRB)
#define MOSI                                    (PB2)
#define SCK                                     (PB1)


#define RTC_CS_PIN                              (PB0)
#define RTC_CS_PORT                             (PORTB)
#define RTC_CS_DDR                              (DDRB)

/* Macros used in driver */
#define CS_HIGH()                               (RTC_CS_PORT |= (1<<RTC_CS_PIN))
#define CS_LOW()                                (RTC_CS_PORT &= ~(1<<RTC_CS_PIN))

#define DS3234_DUMMY_BYTE                       (0xFF)
#define RTC_RESET_PORT  PORTK
#define RTC_RESET_DDR   DDRK
#define RTC_RESET_PIN   0
#define RTC_ENABLE()    RTC_RESET_DDR |= (1<<RTC_RESET_PIN);RTC_RESET_PORT |= (1<<RTC_RESET_PIN)
struct spi_device_t ds3234 =
{
    .init = ds3234_init,
};

void ds3234_init(void)
{
    /* Enable RTC */
    //RTC_ENABLE();
//    /* Set MOSI and SCK output */
    DDR_SPI |= (1<<MOSI) | (1<<SCK);
    /* CS is output */
    RTC_CS_DDR |= (1<<RTC_CS_PIN);
    /* SPI SS pin as output to aviod slave mode */
    DDRB |= (1<<PB0);

    /*
     * Enable SPI, Master, set clock rate.
     * ds3234 can run in 4MHz max
     */
    SPCR = (1<<SPE) /*| (1<<SPIE)*/ | (1<<MSTR) | (1<<SPR1) |(1<<SPR0) | (1<<CPHA);
    //SPSR = (1<<SPI2X);
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

void ds3234_set_time(struct rtc_time *time)
{
    CS_LOW();
    spi_transfer(0x80);
    spi_transfer(time->sec);
    spi_transfer(time->min);
    spi_transfer(time->hour & ~0x40); //Prevent setting 12 hour mode
    spi_transfer(time->day & 0x07);
    spi_transfer(time->date & 0x3F);
    spi_transfer(time->month & 0x1F);
    spi_transfer(time->year);
    CS_HIGH();
}

void ds3234_get_time(struct rtc_time *time)
{
    uint8_t reg;

    CS_LOW();
    spi_transfer(0x00);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->sec = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->min = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->hour = (((reg & 0x30) >> 4) * 10) + (reg & 0x0F);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->day = (reg & 0x07);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->date = (((reg & 0xF0) >> 4) * 10) + (reg & 0x0F);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->month = (((reg & 0x10) >> 4) * 10) + (reg & 0x0F);
    reg = spi_transfer(DS3234_DUMMY_BYTE);
    time->year = (((reg & 0x10) >> 4) * 10) + (reg & 0x0F);
    CS_HIGH();
}
