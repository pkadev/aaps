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
#define IPC_SYNC_BYTE 0xfc
#define IPC_FINALIZE_BYTE 0xc0
#define IPC_GET_BYTE 0x55
#define IPC_PUT_BYTE 0x66
#define WAIT_CNT 15000
#define PUT_WAIT_CNT 15000


struct spi_device_t analog_zero =
{
    //.opto_coupled = true,
    //.hw_ch = system_channel[0],
    .init = init_aaps_a,
};
struct spi_device_t analog_one =
{
    //.opto_coupled = true,
    //.hw_ch = system_channel[0],
    .init = init_aaps_a,
};
volatile uint8_t send_semaphore = 0;
volatile uint8_t spi_data_buf = 0;

ipc_ret_t ipc_get_pkt(uint8_t slave, struct ipc_packet_t *pkt)
{
    /* TODO: Handle return values */
    volatile uint8_t data;
    uint8_t buf = IPC_GET_BYTE;
    uint16_t wait_cnt = WAIT_CNT;
    struct spi_device_t *dev = channel_lookup(slave);

    enable(dev->hw_ch);
    do
    {
        data = spi_transfer(buf);
        if (!wait_cnt--)
        {
            printk("Transfer failed! 0x%x\n", data);
            goto no_answer;
        }
    }while(data != IPC_SYNC_BYTE); /* Wait for ACK */

    /* First byte is data length */
    uint8_t rx_len = spi_transfer(buf);
    pkt->len = rx_len;

    /* Allocate enough bytes for data */
    pkt->data = (uint8_t*)malloc(pkt->len - IPC_PKT_OVERHEAD);
    if (pkt->data == NULL)
        goto no_answer;

    if (pkt->len == 0)
    {
        /* TODO: Add return value */
        printk("Len is zero\n");
        goto no_answer;
    }

    /* TODO: Remove duplication of length variable
     * on slave side.
     */
    data = spi_transfer(buf);
    rx_len--;

    pkt->cmd = spi_transfer(buf);
    rx_len--;

    pkt->crc = spi_transfer(buf);
    rx_len--;

    while(rx_len--)
    {
        pkt->data[rx_len] = spi_transfer(buf);
        /*
         * This delay is dependant on system
         * clocks on master and slave.
         */
        _delay_us(8);
    }

    /* Synchronize end of transmission */
    wait_cnt = WAIT_CNT;
    do
    {
        data = spi_transfer(buf);
        if (!wait_cnt--)
        {
            printk("Finalize failed! Received: 0x%x\n", data);
            goto no_answer;
        }
    }while(data != IPC_FINALIZE_BYTE); /* Wait for ACK */

no_answer:
    irq_from_slave[slave]--;

    if (irq_from_slave[slave] < 0)
        return IPC_RET_ERROR_GENERIC;

    disable(dev->hw_ch);
    return IPC_RET_OK;
}

struct spi_device_t *channel_lookup(uint8_t ch)
{
    switch(ch)
    {
        case 0: { /*printk("ch0\n");*/ return &analog_zero; }
        case 1: { /*printk("ch1\n");*/ return &analog_one; }
        default:
            printk("Error. No such channel [%u]\n", ch);
    }
    return NULL;
}

ipc_ret_t ipc_put_pkt(uint8_t slave, struct ipc_packet_t *pkt)
{
    uint8_t data;
    ipc_ret_t result = IPC_RET_OK;
    uint16_t wait_cnt = PUT_WAIT_CNT;
    struct spi_device_t *dev = channel_lookup(slave);

    if (pkt == NULL || dev == NULL)
        return IPC_RET_ERROR_BAD_PARAMS;

    enable(dev->hw_ch);
    do
    {
        data = spi_transfer(IPC_PUT_BYTE);
        if (!wait_cnt--)
        {
            result = IPC_RET_ERROR_PUT_SYNC;
            goto no_answer;
        }
    }while(data != IPC_SYNC_BYTE); /* Wait for ACK */

    /* Put packet overhead data */
    spi_transfer(pkt->len);
    spi_transfer(pkt->cmd);
    spi_transfer(pkt->crc);

    /* Put packet payload */
    for (uint8_t i = 0; i < pkt->len - IPC_PKT_OVERHEAD; i++)
        spi_transfer(pkt->data[i]);

    wait_cnt = WAIT_CNT;
    do
    {
        /* TODO: Fix this magic number */
        data = spi_transfer(0x10);
        if (!wait_cnt--)
        {
            result = IPC_RET_ERROR_PUT_FINALIZE;
            goto no_answer;
        }
    }while(data != IPC_FINALIZE_BYTE); /* Wait for ACK */

no_answer:

    disable(dev->hw_ch);
    return result;
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
