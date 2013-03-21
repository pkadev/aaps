#ifndef IPC_H_
#define IPC_H_

#include <avr/io.h>
struct ipc_slave_t
{
    uint8_t cs_pin;
};
void ipc_send(struct ipc_slave_t *slave, uint8_t buf);

#endif
