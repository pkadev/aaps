#include "ipc.h"
#include "uart.h"
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>
#include "m128_hal.h"
#include <stdlib.h>
#include <spi.h>
#include "aaps_a.h"

#define IPC_DUMMY_BYTE 0xff

volatile uint8_t spi_data_buf = 0;

//static ipc_ret_t ipc_cmd_invoke(struct spi_device_t *dev, enum ipc_command_t cmd);
static void suspend_irq(void)
{
   // EIMSK &= ~(1<<INT5); //TODO: Platform dependant! Remove!!
}

static void resume_irq(void)
{
 //   EIFR |= (1<<INTF5);//TODO: Platform dependant! Remove!!
 //   EIMSK |= (1<<INT5);//TODO: Platform dependant! Remove!!
}
int8_t ipc_which_irq(volatile int8_t irq_flags[])
{
    /* TODO: Perhaps 'i' should be static so
     * that IRQs in the end of the vector doesn't
     * get starved.
     */
    uint8_t i;
    for (i = 0; i < HW_NBR_OF_CHANNELS; i++)
    {
        if (irq_flags[i])
            return i;
    }
    return NO_IRQ;
}
ipc_ret_t ipc_get_irq_reason(struct spi_device_t *dev, ipc_irq_reason_t *irq_reason)
{
    suspend_irq();
    if (dev == NULL || irq_reason == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    *irq_reason = spi_send_one(dev, IPC_DUMMY_BYTE);

    resume_irq();

    return IPC_RET_OK;
}

// should be static when get wrapper function code is moved from main
ipc_ret_t ipc_get_data_len(struct spi_device_t *dev, uint8_t *len)
{
    if (dev == NULL)
       return IPC_RET_ERROR_BAD_PARAMS;

    suspend_irq();

    /* Get available data length */
    *len = spi_send_one(dev, IPC_DUMMY_BYTE);

    resume_irq();

    return IPC_RET_OK;
}

ipc_ret_t ipc_get_available_data(struct spi_device_t *dev, uint8_t *buf, uint8_t len)
{
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL || buf == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    suspend_irq();

    spi_send_multi(dev, buf, len);
    ret = IPC_RET_OK;

    resume_irq();

    return ret;
}

ipc_ret_t ipc_periph_detect(struct spi_device_t *dev, uint8_t *periph_type)
{
    uint8_t ipc_packet[] =
    {
        IPC_CMD_PERIPH_DETECT,
        0x02,
        0x59,
        0x16,
        0xEF,
    };
    //int16_t retries = 20000; //Found during trial and error
    uint8_t bytes_to_send = 5;
    uint8_t cnter = 0;
    ipc_ret_t ret = IPC_RET_ERROR_GENERIC;

    if (dev == NULL || periph_type == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    suspend_irq();

    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(ipc_packet[cnter++]));
    }
    ret = IPC_RET_OK;
    resume_irq();
    return ret;
}
