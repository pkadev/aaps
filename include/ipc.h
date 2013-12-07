#ifndef IPC_H_
#define IPC_H_
#include <avr/io.h>
#include "spi.h"

/* IPC Commands */
enum ipc_command_t
{
    IPC_CMD_SUPPORTED_CMDS,
    IPC_CMD_PERIPH_DETECT,
    IPC_CMD_NUM_OF_CMDS,
    IPC_CMD_GET_TEMP,
    IPC_CMD_GET_ADC,
    IPC_CMD_DATA_AVAILABLE,
    IPC_CMD_SET_VOLTAGE,
    IPC_CMD_SET_CURRENT_LIMIT,
    IPC_CMD_SET_RELAY_D,
    IPC_CMD_SET_RELAY,
    IPC_CMD_PUT_DATA,
};

enum ipc_data_type_t
{
    IPC_DATA_VOLTAGE = 0x7B,
    IPC_DATA_CURRENT = 0x7C,
    IPC_DATA_ASCII = 0x7D,
    IPC_DATA_THERMO = 0x7E,
    IPC_DATA_ENC = 0x7F,
};
#define IPC_DATA_LEN 2
struct ipc_packet_t
{
    uint8_t len;
    uint8_t cmd;
    uint8_t crc;
    uint8_t *data;
};

typedef uint8_t periph_type_t;
typedef uint8_t ipc_irq_reason_t;
typedef enum
{
    IPC_RET_OK,
    IPC_RET_ERROR_GENERIC,
    IPC_RET_ERROR_BAD_PARAMS,
    IPC_RET_ERROR_TARGET_DEAD,
    IPC_RET_ERROR_RX,
    IPC_RET_ERROR_TX,
    IPC_RET_ERROR_NOT_SUPPORTED,
    IPC_RET_ERROR_PUT_SYNC,
    IPC_RET_ERROR_GET_SYNC,
    IPC_RET_ERROR_PUT_FINALIZE,
    IPC_RET_ERROR_GET_FINALIZE,
    IPC_RET_ERROR_TX_BUF_EMPTY,
    IPC_RET_ERROR_OUT_OF_MEMORY,
} ipc_ret_t;

struct spi_device_t *channel_lookup(uint8_t ch);
extern struct spi_device_t analog_zero;
extern struct spi_device_t analog_one;
ipc_ret_t ipc_get_pkt(uint8_t slave, struct ipc_packet_t *pkt);
ipc_ret_t ipc_put_pkt(uint8_t slave, struct ipc_packet_t *pkt);
ipc_ret_t ipc_periph_detect(struct spi_device_t *dev, uint8_t *periph_type);
int8_t ipc_which_irq(volatile int8_t irq_flags[]);

#define IPC_PKT_OVERHEAD 3  /* Len, cmd, crc */

#endif
