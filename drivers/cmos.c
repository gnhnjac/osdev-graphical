#include "cmos.h"
#include "low_level.h"

void cmos_NMI_enable() {
	outb(0x70, inb(0x70) & 0x7F);
	inb(0x71);
}
 
void cmos_NMI_disable() {
	outb(0x70, inb(0x70) | 0x80);
	inb(0x71);
}

uint8_t cmos_NMI_get()
{

	return inb(0x70) & 0x80;

}

void cmos_rtc_wait_for_update()
{

	// wait for it to set
	while(!(cmos_read_register(CMOS_REG_STAT_A)&0b1000000))
		continue;

	// wait for it to clear
	while(cmos_read_register(CMOS_REG_STAT_A)&0b1000000)
		continue;

}

// function doesn't work
uint8_t cmos_rtc_read(uint8_t reg)
{

	cmos_rtc_wait_for_update();

	outb(0x70, cmos_NMI_get()|reg);

	return inb(0x71);

}

uint8_t cmos_read_register(uint8_t reg)
{

	outb(0x70, cmos_NMI_get()|reg);

	return inb(0x71);

}

rtc_time_fmt cmos_rtc_get_fmt()
{

	uint8_t b_reg = cmos_read_register(CMOS_REG_STAT_B);

	if (b_reg&2)
		return RTC_24HOUR;

	return RTC_12HOUR;

}

rtc_mode cmos_rtc_get_mode()
{

	uint8_t b_reg = cmos_read_register(CMOS_REG_STAT_B);

	if (b_reg&4)
		return RTC_BIN_MODE;

	return RTC_BCD_MODE;

}

uint8_t cmos_rtc_get_seconds()
{

	if (cmos_rtc_get_mode() == RTC_BIN_MODE)
		return cmos_read_register(CMOS_REG_SECONDS);
	else
	{

		uint8_t bcd = cmos_read_register(CMOS_REG_SECONDS);
		return ( (bcd & 0xF0) >> 1) + ( (bcd & 0xF0) >> 3) + (bcd & 0xf);

	}

}

uint8_t cmos_rtc_get_minutes()
{

	if (cmos_rtc_get_mode() == RTC_BIN_MODE)
		return cmos_read_register(CMOS_REG_MINUTES);
	else
	{

		uint8_t bcd = cmos_read_register(CMOS_REG_MINUTES);
		return ( (bcd & 0xF0) >> 1) + ( (bcd & 0xF0) >> 3) + (bcd & 0xf);

	}

}

uint8_t cmos_rtc_get_hour()
{

	uint8_t hour = cmos_read_register(CMOS_REG_HOURS);

	if (cmos_rtc_get_fmt() == RTC_12HOUR)
		hour &= 0x7F;

	if (cmos_rtc_get_mode() == RTC_BIN_MODE)
		return hour;
	else
		return ( (hour & 0xF0) >> 1) + ( (hour & 0xF0) >> 3) + (hour & 0xf);

}