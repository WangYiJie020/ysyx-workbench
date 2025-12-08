#include <am.h>
#define RTC_ADDR 0x10000048

static uint64_t _am_start_time;
static uint64_t get_us_time() {
  uint64_t lo = *(volatile uint32_t *)(RTC_ADDR);
  uint64_t hi = *(volatile uint32_t *)(RTC_ADDR + 4);
  uint64_t res = (hi << 32) | lo;
  return res;
}

void __am_timer_init() { _am_start_time = get_us_time(); }

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = get_us_time() - _am_start_time;
}

static bool is_leap(int year) {
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static const int days_before_month[2][12] = {
    // non-leap year
    {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
    // leap year
    {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}};

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  uint64_t t = get_us_time();

  uint64_t total_seconds = t / 1000000;
  uint64_t total_days = total_seconds / 86400;
  uint64_t seconds_in_day = total_seconds % 86400;

  rtc->second = seconds_in_day % 60;
  rtc->minute = (seconds_in_day / 60) % 60;
  rtc->hour = (seconds_in_day / 3600) % 24;

  int year = 1970;
  while (true) {
    int days_in_year = is_leap(year) ? 366 : 365;
    if (total_days >= days_in_year) {
      total_days -= days_in_year;
      year++;
    } else {
      break;
    }
  }
  rtc->year = year;

  bool leap = is_leap(year);
  int month = 0;
  while (month < 12 && total_days >= days_before_month[leap][month + 1]) {
    month++;
  }

	rtc->month = month + 1;
	rtc->day = total_days - days_before_month[leap][month] + 1;

  /*
rtc->second = get_us_time()/1000000;
rtc->minute = 0;
rtc->hour   = 0;
rtc->day    = 0;
rtc->month  = 0;
rtc->year   = 1900;
*/
}
