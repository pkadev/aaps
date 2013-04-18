

#ifndef DS3234_H__
#define DS3234_H__

struct rtc_time
{
    uint16_t year;
    uint8_t month;
    uint8_t date;
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

enum
{
    ERROR_NO_ERROR = 0,
    ERROR_INVALID_CHANNEL,
    ERROR_INVALID_CLOCK_MODE
}ds3234_error_t;

void ds3234_init(void);


uint8_t ds3234_read_ctrl_reg(void);
void ds3234_write_reg(uint8_t reg, uint8_t val);
void ds3234_get_time(struct rtc_time *time);
#endif
