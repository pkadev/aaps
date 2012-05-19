#ifndef ONE_WIRE_H__
#define ONE_WIRE_H__

#include <util/delay.h>

#define FALSE 0
#define TRUE  1
#define DQ_PORT PORTE
#define DQ_DDR DDRE
#define DQ_PIN 7
#define DQ_IN PINE
#define DQ_MASK 0x80
#define DQ_LOW() PORTE&=~(1<<DQ_PIN)
#define DQ_HIGH() PORTE|=(1<<DQ_PIN)

#define CRC8INIT    0x00
#define CRC8POLY    0x18    //0X18 = X^8+X^5+X^4+X^0

#define OW_ROM_BIT_LEN 64
#define OW_ROM_BYTE_LEN 8
#define THERM_DECIMAL_STEPS_12BIT 625

/*
 * One Wire Command defines
 */
#define OW_CMDF0_SEARCH_ROM 0xF0
#define OW_CMD33_READ_ROM 0x33
#define OW_CMD55_MATCH_ROM 0x55
#define OW_CMD44_CONV_TEMP 0x44
#define OW_CMDCC_SKIP_ROM 0xCC
#define OW_CMDBE_READ_SCRATCHPAD 0xBE

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
ow_ret_val_t ow_get_devices(ow_device_t *ow_devices);
ow_ret_val_t ow_read_temperature(ow_device_t *ow_device, ow_temp_t *ow_temp);
ow_ret_val_t ow_read_scratchpad(ow_device_t *rom, ow_scratchpad_t *scratchpad);

#endif

