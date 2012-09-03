#ifndef M128_HAL_H__
#define M128_HAL_H__
#include <avr/io.h>
/*
 * AVR specific defines 
 */


/*
 * System defines 
 */
#define STATUS_REGISTER         SREG
#define STATUS_REGISTER_IT      SREG_I
/*
 * Defines for Watchdog
 */
#define WD_CTRL_REG             WDTCSR
#define WD_CHANGE_ENABLE        WDCE
#define WD_IT_ENABLE_MASK       WDIE
#define WD_ENABLE               WDE
#define WD_PRESCALER2           WDP2
#define WD_PRESCALER1           WDP1
#define WD_PRESCALER0           WDP0

/*
 * UART0 defines
 */
#define UART_DATA_REG           UDR0
#define UART_BAUD_RATE_REG_LOW  UBRR0L

#endif
