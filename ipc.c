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

volatile uint8_t send_semaphore = 0;
volatile uint8_t spi_data_buf = 0;

ipc_ret_t ipc_transfer_raw(struct spi_device_t *dev, uint8_t slave)
{
    /* TODO: Handle return values */
    /* TODO: Define ack and finalize variables 0xfc 0xc0 */

#define WAIT_CNT 2000
    uint8_t buf = 0x55;
    uint8_t recv = 0;
    uint8_t data;

    enable(dev->hw_ch);
    uint16_t wait_cnt = WAIT_CNT;
    do
    {
        recv = spi_transfer(buf);
        //printk("trying...0x%x\n", recv);
        if (!wait_cnt--)
        {
            printk("Transfer failed! 0x%x\n", recv);
            goto no_answer;
        }
    }while(recv != 0xfc); /* Wait for ACK */

    /* First byte is data length */
    uint8_t rx_len = spi_transfer(buf);

    if (rx_len == 0)
        printk("Len is zero\n");

    while(rx_len--)
    {
        data =  spi_transfer(buf);

        /*
         * This delay is dependant on system
         * clocks on master and slave.
         */
        _delay_us(2); 
    }

    /* Synchronize end of transmission */
    wait_cnt = WAIT_CNT;
    do
    {
        recv = spi_transfer(buf);
        //printk("Finalizing...0x%x\n", recv);
        if (!wait_cnt--)
        {
            printk("Finalize failed! Received: 0x%x\n", recv);
            goto no_answer;
        }
    }while(recv != 0xC0); /* Wait for ACK */
no_answer:
    disable(dev->hw_ch);
    irq_from_slave[slave]--;

    if (irq_from_slave[slave] < 0)
        return IPC_RET_ERROR_GENERIC;

    return IPC_RET_OK;
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

    while(bytes_to_send--)
    {
        init_aaps_a(dev->hw_ch);
        spi_send_one(dev, ~(ipc_packet[cnter++]));
    }
    ret = IPC_RET_OK;
    return ret;
}
