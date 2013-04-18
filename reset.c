#include <avr/io.h>    
#include <stdint.h>
#include "reset.h"
#include "uart.h"

#define RST_REASON_MSK      0x0F
#define RST_WDT             0x08
#define RST_BO              0x04
#define RST_EXT             0x02
#define RST_POWERON         0x01
#define RST_NUM_REASONS     4

static volatile uint8_t rst_reason;

static const char *get_reason(uint8_t reason)
{
    switch(reason)
    {
        case RST_WDT: return "WATCHDOG";
        case RST_BO: return "BROWN-OUT";
        case RST_EXT: return "EXTERNAL";
        case RST_POWERON: return "POWER-ON";
    }
    return "UNKNOWN";
}

void rst_save_reason(void)
{
    rst_reason = MCUSR;
    MCUSR = 0x00;
}

void rst_print_reason(void)
{
    printk("Reset reason: %s\n", get_reason(rst_reason));
}
