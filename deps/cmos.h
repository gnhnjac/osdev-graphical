#include <stdint.h>

#define CMOS_REG_SECONDS 0
#define CMOS_REG_MINUTES 2
#define CMOS_REG_HOURS 4
#define CMOS_REG_STAT_A 0x0A
#define CMOS_REG_STAT_B 0x0B

typedef enum _rtc_time_fmt
{
	RTC_24HOUR = 1,
	RTC_12HOUR = 2
} rtc_time_fmt;

typedef enum _rtc_mode
{

	RTC_BCD_MODE = 1,
	RTC_BIN_MODE = 2

} rtc_mode;

//refs
void cmos_NMI_enable();
void cmos_NMI_disable();
uint8_t cmos_NMI_get();
void cmos_rtc_wait_for_update();
uint8_t cmos_rtc_read(uint8_t reg);
uint8_t cmos_read_register(uint8_t reg);
rtc_time_fmt cmos_rtc_get_fmt();
rtc_mode cmos_rtc_get_mode();
uint8_t cmos_rtc_get_seconds();
uint8_t cmos_rtc_get_minutes();
uint8_t cmos_rtc_get_hour();