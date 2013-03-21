#include "ipc.h"
#include "uart.h"
#include <util/delay.h>
#include <avr/interrupt.h>

#define CS_M48 PE2
#define CS_M48_LOW() (PORTE&=~(1<<CS_M48))
#define CS_M48_HIGH() (PORTE|=(1<<CS_M48))
#define IPC_CS_HIGH(pin) (PORTE |= (1<<pin))
#define IPC_CS_LOW(pin) (PORTE &= ~(1<<pin))
#define SPI_WAIT()  while(!(SPSR & (1<<SPIF)))

//#define CS_M48_TGL() (PORTE^=(1<<CS_M48))

//ISR(SPI_STC_vect)
//{   
//    printk("SPDR 0x%x\n", SPDR);
//    printk("SPSR 0x%x\n", SPSR);
//}

volatile uint8_t spi_data_buf = 0;

static uint8_t ipc_xfer_byte(uint8_t tx)
{
    _delay_us(75); //this is a hack. Why is it needed?
    SPDR = tx;
    if(!(SPCR & (1<<SPIE)))
        SPI_WAIT();

    return SPDR;
}

void ipc_send(struct ipc_slave_t *slave, uint8_t buf)
{
    IPC_CS_LOW(slave->cs_pin);
    _delay_us(10);
    ipc_xfer_byte(buf);
    _delay_us(20);
    IPC_CS_HIGH(slave->cs_pin);
}
