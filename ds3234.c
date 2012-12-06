#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>
#include "uart.h"
#include "ds3234.h"

void _swap(uint8_t* data)
{
    uint8_t storage = 0;
    storage = (*data<<4);
    *data >>= 4;
    *data |= storage;
}

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
    SPCR = (1<<SPE) | (1<<MSTR) | (1<<CPHA);
    CS_HIGH();
}

static uint8_t ds3234_xfer_byte(uint8_t tx)
{
    uint8_t val;
    SPDR = tx;
    SPI_WAIT();
    val = SPDR;
    return val;
}


void ds3234_write_reg(uint8_t val)
{
    CS_LOW();
    ds3234_xfer_byte(0x8e);
    ds3234_xfer_byte(val);
    CS_HIGH();
}
uint8_t ds3234_read_ctrl_reg()
{
    uint8_t val = 0;

    CS_LOW();
    ds3234_xfer_byte(0x0e);
    val = ds3234_xfer_byte(DS3234_DUMMY_BYTE);
    CS_HIGH();
    return val; 
}

