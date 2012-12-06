

#ifndef DS3234_H__
#define DS3234_H__

struct rtc_time 
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

/** hardware specific */
#define DDR_SPI                                 (DDRB)
#define MOSI                                    (PB2)
#define SCK                                     (PB1)

#define RTC_CS_PIN                              (PG3)
#define RTC_CS_PORT                             (PORTG)
#define RTC_CS_DDR                              (DDRG)

#define DS3234_DUMMY_BYTE                       (0xFF)


/* Macros used in driver */
#define CS_HIGH()                               (RTC_CS_PORT |= (1<<RTC_CS_PIN))
#define CS_LOW()                                (RTC_CS_PORT &= ~(1<<RTC_CS_PIN))
#define SPI_WAIT()                              while(!(SPSR & (1<<SPIF)))

enum
{
    ERROR_NO_ERROR = 0,
    ERROR_INVALID_CHANNEL,
    ERROR_INVALID_CLOCK_MODE
}ds3234_error_t;


/***********************************************************************
 * Author:          Ke2
 *
 * Date:            09-10-14
 *
 * Function: 	    ds3234_init
 *
 * param[in]:   	void - Need better setup SPI Clock etc.
 *
 * Returns:	        uint8_t - 
 *                  --- NOT IMPLEMENTED - ALWAYS RETURNS NONZERO ---
 *
 * Description: 	Initializes ds3234. Run before any other
 *                  calls to ds3234.
 *
 ***********************************************************************/

void ds3234_init(void);



/***********************************************************************
 * Author:          Ke2
 *
 * Date:            09-10-14
 *
 * Function: 	    ds3234_send_byte
 *
 * param[in]:   	uint8_t - Data that will be sent to SPI slave
 *
 * Returns:	        uint8_t - nonzero indicates success. Zero on failure.
 *                  --- NOT IMPLEMENTED - ALWAYS RETURNS NONZERO ---
 *
 * Description: 	Send a byte to ds3234.
 *
 ***********************************************************************/
void max1168_send_byte(uint8_t cData);

uint8_t ds3234_read_ctrl_reg(void);
void ds3234_write_reg(uint8_t val);
#endif
