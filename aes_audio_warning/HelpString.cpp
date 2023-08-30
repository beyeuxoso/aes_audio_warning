
#include "HelpString.h"

String getIndex(String data, String strIndex) {
	return data.substring(0, data.indexOf(strIndex));
}
String getLastIndex(String data, String strIndex) {
	return data.substring(0, data.lastIndexOf(strIndex));
}
String trimIndex(String data, String strIndex) {
	data.remove(0, data.indexOf(strIndex) + 1); data.trim();
	return data;
}
String trimLastIndex(String data, String strIndex) {
	data.remove(0, data.lastIndexOf(strIndex) + 1); data.trim();
	return data;
}
// Return DateTime in format: "year-month-day hour:minute:second"
String DateTimeToString(tm tmstruct) {
	return String((tmstruct.tm_year)) + "-" + String((tmstruct.tm_mon) + 1) + "-" + String(tmstruct.tm_mday) + " " + String(tmstruct.tm_hour) + ":" + String(tmstruct.tm_min) + ":" + String(tmstruct.tm_sec);
}

double longTime(String data) {
	data.replace(":", "");
	return data.toDouble();
}