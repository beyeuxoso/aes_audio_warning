#ifndef __MIDDLEWARES_RTC_DS3231_H_
#define __MIDDLEWARES_RTC_DS3231_H_

#include <time.h>

//Địa chỉ của thời gian thực
#define DS3231_I2C_ADDRESS				(0x68)

//Dành cho thời gian thực
extern String datetime;
//Thời gian khởi động chương trình
extern String StartUp_datetime;
//Tách chỉ hiển thị ngày
extern String strdate;
//Tách chỉ hiển thị giờ:phút:giây
extern String strtime;

//extern String DateTimeToString(tm tmstruct);
//extern tm RTC_DS3231_Get();
//extern void RTC_DS3231_Set(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t dayOfMonth, uint8_t month, uint8_t year);
extern tm RTC_DS3231_Get();
extern void RTC_DS3231_Set(tm* time);
extern void RTC_DS3231_Main();

#endif	/* __MIDDLEWARES_RTC_DS3231_H_ */