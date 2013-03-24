#include "ipc.h"
#include "uart.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "m128_hal.h"
#include <stdlib.h>

#define IPC_CS_HIGH(pin) (PORTE |= (1<<pin))
#define IPC_CS_LOW(pin) (PORTE &= ~(1<<pin))
#define SPI_WAIT() while(!(SPSR & (1<<SPIF)))

#define IPC_DUMMY_BYTE 0xff

volatile uint8_t spi_data_buf = 0;

static ipc_ret_t ipc_cmd_invoke(struct ipc_slave_t *dev, enum ipc_command_t cmd);

ipc_ret_t ipc_periph_detect(struct ipc_slave_t *dev, uint8_t *periph_type)
{
    int16_t retries = 20000; //Found during trial and error
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL || periph_type == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    EIMSK &= ~(1<<INT5); //TODO: Platform dependant! Remove!!
    ret = ipc_cmd_invoke(dev, IPC_CMD_PERIPH_DETECT);

    /* Wait for slave to provide data and signal */
    while ((IRQ_FUTUR_IN & (1<<IRQ_FUTUR_PIN)) != (1<<IRQ_FUTUR_PIN) && retries--);

    if (retries == 0) {
        ret = IPC_RET_ERROR_TARGET_DEAD;
        goto exit;
    }

    /* Slave is ready, clock data out */
    *periph_type = ipc_send_one(dev, IPC_DUMMY_BYTE);
    ret = IPC_RET_OK;

exit:
    EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
    EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!

    return ret;
}

static ipc_ret_t ipc_cmd_invoke(struct ipc_slave_t *dev, enum ipc_command_t cmd)
{
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    ipc_send_one(dev, cmd);

    ret = IPC_RET_OK;
    return ret;
}

static uint8_t ipc_xfer_byte(uint8_t tx)
{
    _delay_us(75); //this is a hack. Why is it needed?
    SPDR = tx;
    if(!(SPCR & (1<<SPIE)))
        SPI_WAIT();

    return SPDR;
}

uint8_t ipc_send_one(struct ipc_slave_t *slave, uint8_t buf)
{
    uint8_t recv = 0;

    IPC_CS_LOW(slave->cs_pin);
    _delay_us(10);
    recv = ipc_xfer_byte(buf);
    _delay_us(20);

    IPC_CS_HIGH(slave->cs_pin);
    return recv;
}
