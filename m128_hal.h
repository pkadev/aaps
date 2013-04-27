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
#define UART_DATA_REG           UDR2
#define UART_BAUD_RATE_REG_LOW  UBRR2L

#define IRQ_FUTUR_PIN 5
#define IRQ_FUTUR_IN PINE
#define IRQ_FUTUR_DDR DDRE
#define IRQ_FUTUR_PORT PORTE
#define WAIT_FOR_CS_FUTURE_LOW() while ((IRQ_FUTUR_IN & (1<<IRQ_FUTUR_PIN)) == (1<<IRQ_FUTUR_PIN))
#define WAIT_FOR_CS_FUTURE_HIGH() while ((IRQ_FUTUR_IN & (1<<IRQ_FUTUR_PIN)) != (1<<IRQ_FUTUR_PIN))
#endif
