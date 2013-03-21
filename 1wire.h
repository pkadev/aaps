#ifndef ONE_WIRE_H__
#define ONE_WIRE_H__

#include <util/delay.h>

#define OW_ROM_BYTE_LEN 8


typedef enum {
    OW_RET_FAIL,
    OW_RET_OK,
} ow_ret_val_t;

typedef struct {
    uint8_t addr[OW_ROM_BYTE_LEN]; 
}ow_device_t;

typedef struct {
    uint8_t data[9];
}ow_scratchpad_t;

typedef struct {
    uint16_t temp;
    uint16_t dec;
}ow_temp_t;

/*
 * Public function for 1-Wire
 */
uint8_t ow_num_devices();
void ow_print_device_addr(ow_device_t *ow_device);
ow_ret_val_t ow_get_devices(ow_device_t *ow_devices);
ow_ret_val_t ow_read_temperature(ow_device_t *ow_device, ow_temp_t *ow_temp);
ow_ret_val_t ow_read_scratchpad(ow_device_t *rom, ow_scratchpad_t *scratchpad);

#endif

