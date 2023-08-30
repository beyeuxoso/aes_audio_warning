
#include <Wire.h>
#include "RTC_DS3231.h"
#include "HelpString.h"
#include "Dbg.h"

String datetime = "";				//Dành cho thời gian thực
String StartUp_datetime = "";		//Thời gian khởi động chương trình
String strdate = "";				//Tách chỉ hiển thị ngày
String strtime = "";				//Tách chỉ hiển thị giờ:phút:giây

// Convert normal decimal numbers to binary coded decimal
static inline uint8_t decToBcd(uint8_t val) {
	return((val / 10 * 16) + (val % 10));
}
// Convert binary coded decimal to normal decimal numbers
static inline uint8_t bcdToDec(uint8_t val) {
	return((val / 16 * 10) + (val % 16));
}

void RTC_DS3231_Set(tm* time) {
	// sets time and date data to DS3231
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0);									// set next input to start at the seconds register
	Wire.write(decToBcd(time->tm_sec));				// set seconds
	Wire.write(decToBcd(time->tm_min));				// set minutes
	Wire.write(decToBcd(time->tm_hour));			// set hours
	Wire.write(decToBcd(1));						// set day of week (1=Sunday, 7=Saturday)
	Wire.write(decToBcd(time->tm_mday));			// set date (1 to 31)
	Wire.write(decToBcd(time->tm_mon));				// set month
	Wire.write(decToBcd(time->tm_year - 2000));		// set year (0 to 99)
	Wire.endTransmission();
}

tm RTC_DS3231_Get() {
	tm tmstruct;
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0);									// set DS3231 register pointer to 00h 
	uint8_t error = Wire.endTransmission();
	if (error == 4) {
		return tmstruct;
	}
	Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
	// Request seven bytes of data from DS3231 starting from register 00h
	tmstruct.tm_sec = bcdToDec(Wire.read() & 0x7f);
	tmstruct.tm_min = bcdToDec(Wire.read());
	tmstruct.tm_hour = bcdToDec(Wire.read() & 0x3f);
	tmstruct.tm_wday = bcdToDec(Wire.read());
	tmstruct.tm_mday = bcdToDec(Wire.read());
	tmstruct.tm_mon = bcdToDec(Wire.read());
	tmstruct.tm_year = bcdToDec(Wire.read()) + 2000;
	datetime = String(tmstruct.tm_year) + "-" + String(tmstruct.tm_mon).padleft(2, "0") + "-" + String(tmstruct.tm_mday).padleft(2, "0") + " " +
		String(tmstruct.tm_hour).padleft(2, "0") + ":" + String(tmstruct.tm_min).padleft(2, "0") + ":" + String(tmstruct.tm_sec).padleft(2, "0");
	strdate = getIndex(datetime, " ");
	strtime = datetime.substring(datetime.indexOf(" "));
	if (StartUp_datetime == "") StartUp_datetime = datetime;
	return tmstruct;
}

void RTC_DS3231_Main()
{
	RTC_DS3231_Get();
}