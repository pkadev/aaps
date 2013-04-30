#ifndef DS3234_H__
#define DS3234_H__

struct rtc_time
{
    uint8_t year;
    uint8_t month;
    uint8_t date;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

enum
{
    ERROR_NO_ERROR = 0,
    ERROR_INVALID_CHANNEL,
    ERROR_INVALID_CLOCK_MODE
}ds3234_error_t;

void ds3234_init(void);
uint8_t ds3234_read_ctrl_reg(void);
void ds3234_write_reg(uint8_t reg, uint8_t val);
void ds3234_set_time(struct rtc_time *time);
void ds3234_get_time(struct rtc_time *time);
#endif
