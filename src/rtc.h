#ifndef RTC_H
#define RTC_H

#include "common.h"

u8int rtc_get_year();
u8int rtc_get_month();
u8int rtc_get_day();
u8int rtc_get_weekday();
u8int rtc_get_hour();
u8int rtc_get_minute();
u8int rtc_get_second();
void rtc_print_time();

#endif //RTC_H
