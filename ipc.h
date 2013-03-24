#ifndef IPC_H_
#define IPC_H_
#include <avr/io.h>

/* IPC Commands */
enum ipc_command_t
{
    IPC_CMD_SUPPORTED_CMDS,
    IPC_CMD_PERIPH_DETECT,
    IPC_CMD_NUM_OF_CMDS,
    IPC_CMD_GET_TEMP,
    IPC_CMD_GET_VOLTAGE,
    IPC_CMD_GET_CURRENT
};

struct ipc_data_t
{
    enum ipc_command_t cmd;
    void *buf;
};

typedef uint8_t periph_type_t;

typedef enum
{
    IPC_RET_OK,
    IPC_RET_ERROR_GENERIC,
    IPC_RET_ERROR_BAD_PARAMS,
    IPC_RET_ERROR_TARGET_DEAD,
    IPC_RET_ERROR_NOT_SUPPORTED,
} ipc_ret_t;

struct ipc_slave_t
{
    uint8_t cs_pin;
};

uint8_t ipc_send_one(struct ipc_slave_t *slave, uint8_t buf); //TODO: make this function static and always call wrappers with this function inside them.

ipc_ret_t ipc_periph_detect(struct ipc_slave_t *dev, uint8_t *periph_type);

#endif
