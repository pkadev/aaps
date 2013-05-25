#include "ipc.h"
#include "uart.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "m128_hal.h"
#include <stdlib.h>
#include <spi.h>

#define IPC_DUMMY_BYTE 0xff

volatile uint8_t spi_data_buf = 0;

static ipc_ret_t ipc_cmd_invoke(struct spi_device_t *dev, enum ipc_command_t cmd);

ipc_ret_t ipc_get_irq_reason(struct spi_device_t *dev, ipc_irq_reason_t *irq_reason)
{
    EIMSK &= ~(1<<INT5); //TODO: Platform dependant! Remove!!

    if (dev == NULL || irq_reason == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    *irq_reason = spi_send_one(dev, IPC_DUMMY_BYTE);

    EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
    EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!

    return IPC_RET_OK;
}

// should be static when get wrapper function code is moved from main
ipc_ret_t ipc_get_data_len(struct spi_device_t *dev, uint8_t *len)
{
    if (dev == NULL)
       return IPC_RET_ERROR_BAD_PARAMS;

    EIMSK &= ~(1<<INT5); //TODO: Platform dependant! Remove!!

    /* Get available data length */
    *len = spi_send_one(dev, IPC_DUMMY_BYTE);

    EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
    EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!

    return IPC_RET_OK;
}

ipc_ret_t ipc_get_available_data(struct spi_device_t *dev, uint8_t *buf, uint8_t len)
{
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL || buf == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    EIMSK &= ~(1<<INT5); //TODO: Platform dependant! Remove!!
    spi_send_multi(dev, buf, len);
    ret = IPC_RET_OK;
    EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
    EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!

    return ret;
}

ipc_ret_t ipc_periph_detect(struct spi_device_t *dev, uint8_t *periph_type)
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
    *periph_type = spi_send_one(dev, IPC_DUMMY_BYTE);
    if (*periph_type < 100)
        ret = IPC_RET_ERROR_TARGET_DEAD;
    else
        ret = IPC_RET_OK;

exit:
    EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
    EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!

    return ret;
}

static ipc_ret_t ipc_cmd_invoke(struct spi_device_t *dev, enum ipc_command_t cmd)
{
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    spi_send_one(dev, cmd);

    ret = IPC_RET_OK;
    return ret;
}
