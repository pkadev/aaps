#ifndef UART_H__
#define UART_H__

/**
	Global defines for driver
*/

#define RS232_PRINT_BUFFER 80

/*
 * @brief 	initializes uart0
 */
void uart_init(void);

/*
 * @brief print string to serial port
 */
int printk (const char *fmt, ...);

/*
 * @brief print string to serial port
 */

void print(char* c);


#endif /* UART_H__ */
