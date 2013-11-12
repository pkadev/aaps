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

struct ipc_data_t
{
    enum ipc_command_t cmd;
    void *buf;
};

typedef uint8_t periph_type_t;
typedef uint8_t ipc_irq_reason_t;
typedef enum
{
    IPC_RET_OK,
    IPC_RET_ERROR_GENERIC,
    IPC_RET_ERROR_BAD_PARAMS,
    IPC_RET_ERROR_TARGET_DEAD,
    IPC_RET_ERROR_NOT_SUPPORTED,
} ipc_ret_t;

ipc_ret_t ipc_get_data_len(struct spi_device_t *dev, uint8_t *len);
ipc_ret_t ipc_periph_detect(struct spi_device_t *dev, uint8_t *periph_type);
ipc_ret_t ipc_get_available_data(struct spi_device_t *dev, uint8_t *buf, uint8_t len);
ipc_ret_t ipc_get_irq_reason(struct spi_device_t *dev, ipc_irq_reason_t *irq_reason);
int8_t ipc_which_irq(volatile int8_t irq_flags[]);

#endif
