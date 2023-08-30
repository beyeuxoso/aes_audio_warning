
#include "Dxdiag.h"
#include "Config.h"
#include "RTC_DS3231.h"
#include "SDCard.h"
#include "Memory.h"

String publish_Time = __DATE__;

String GetTypeDevice(String data) {
	//String devicename = *memo.GetValue("DEVICE_NAME");
	//if (devicename == "") devicename = "No name";
	String devName = SDCard_LoadCfg(SD_DEVICE_NAME);
	if (devName == "") devName = "NoName";
	return devName + "/" + String(ESP.getChipId()) + "/10eba031-2203-4105-b80b-04b675dfe576";
}
String ICACHE_FLASH_ATTR DXDIAG_Peripheral() {
	//String data = "";
	//String solidLine = F("\r\n____________________");
	//data += String(F("\r\nVersion Firmware : ")) + (TYPE_DEVICE == 1 ? "full" : (TYPE_DEVICE == 2 ? "lite-i2c" : "lite"));// String(TYPE_DEVICE) + String(F(" (0-> lite, 1-> full)"));
	//data += String(F("\r\nVersion          : ")) + VERSION;
	//data += String(F("\r\nUsing SDCard     : ")) + (USING_SDCARD == 1 ? "Ready" : "Unavailable");
	//data += String(F("\r\nUsing LCD        : ")) + (USING_LCD == 1 ? "Ready" : "Unavailable");
	//data += String(F("\r\nUsing SPIFFS     : ")) + (USING_FS == 1 ? "Ready" : "Unavailable");
	//data += String(F("\r\nUsing ADS 16 bits: ")) + (USING_ADS == 1 ? "Ready" : "Unavailable");
	//data += String(F("\r\nUsing AC 100A    : ")) + (USING_AC100 == 1 ? "Ready" : "Unavailable");
	//data += String(F("\r\nRTC              : ")) + datetime;
	//return data;
	return "";
}
//String ICACHE_FLASH_ATTR DXDIAG_Stack() {
//	String data = "";
//	mystack = LinkedList<objectStack>();
//	for (uint8_t i = 0; i < 20; i++)
//	{
//		objectStack os1;
//		os1.ind = i;
//		os1.result = "This is list indexed " + String(i) + ", RAM size: " + String(ESP.getFreeHeap()) + " kb";
//		mystack.add(os1);
//	}
//	mystack.remove(1);//10 7  3 1
//	mystack.remove(1);
//	mystack.remove(5);
//	mystack.remove(7);
//	String solidLine = F("\r\n____________________");
//	for (uint8_t i = 0; i < mystack.size(); i++) {
//		data += "\r\nStack: " + String(mystack.get(i).result);
//	}
//	data += solidLine;
//	return data;
//}
String ICACHE_FLASH_ATTR DXDIAG(String dat) {
	String data = "";
	float flashChipSize = (float)ESP.getFlashChipSize() / 1024.0 / 1024.0;
	float realFlashChipSize = (float)ESP.getFlashChipRealSize() / 1024.0 / 1024.0;
	float flashFreq = (float)ESP.getFlashChipSpeed() / 1000.0 / 1000.0;
	String stateRSSI = "";
	tm dtime = cvt_date(__DATE__);
	String  dt = String(dtime.tm_year) + "-" + String(dtime.tm_mon).padleft(2, "0") + "-" + String(dtime.tm_mday).padleft(2, "0") + " " __TIME__;
	uint8_t g = GetRSSIState(WiFi.RSSI());
	if (g == 1) stateRSSI = "Expert";
	if (g == 2) stateRSSI = "Very good";
	if (g == 3) stateRSSI = "Good";
	if (g == 4) stateRSSI = "Not good";
	if (g == 5) stateRSSI = "Timeout";
	String solidLine = F("\r\n____________________");
	data += String(F("\r\nFlash chipID      : ")) + String(ESP.getFlashChipId());
	data += String(F("\r\nFlash size total  : ")) + String(flashChipSize) + "Mb";// +String(realFlashChipSize) + "Kb";
	//data += "\r\nFlash size current: " + String(realFlashChipSize) + "Mb / " + String(fileUsedKB) + "Kb";
	data += String(F("\r\nFlash speed       : ")) + String(flashFreq, 0) + "Mhz";
	data += solidLine;
	data += String(F("\r\nCore version      : ")) + ESP.getCoreVersion();
	data += String(F("\r\nChip ID           : ")) + String(ESP.getChipId());
	data += String(F("\r\nCPU Freq          : ")) + String(ESP.getCpuFreqMHz()) + "Mhz";
	data += String(F("\r\nIRAM free         : ")) + String(ESP.getFreeHeap()) + " bytes";
	//data += "\r\nBoot mode         : " + String(ESP.getBootMode());
	data += String(F("\r\nCycle count       : ")) + String(ESP.getCycleCount());
	data += String(F("\r\nFull Version      : ")) + String(ESP.getFullVersion());
	data += String(F("\r\nUsed/Free Sketch  : ")) + String(ESP.getSketchSize() / 1024) + "Kb / " + String(((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000) / 1024) + "Kb";
	//data += "\r\nFree sketch size  : " + String(((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000) / 1024) + "Kb";
	data += solidLine;
	data += String(F("\r\nMac Address       : ")) + WiFi.macAddress();
	data += String(F("\r\nMac Address remote: ")) + WiFi.BSSIDstr();
	data += String(F("\r\nIP Address        : ")) + WiFi.localIP().toString();
	data += String(F("\r\nSubnet mask       : ")) + WiFi.subnetMask().toString();
	data += String(F("\r\nGateway           : ")) + WiFi.gatewayIP().toString();
	data += String(F("\r\nSSID              : ")) + WiFi.SSID();
	data += String(F("\r\nSignal(RSSI)      : ")) + String(WiFi.RSSI()) + "[" + stateRSSI + "]";
	data += String(F("\r\nHostname          : ")) + WiFi.hostname();
	data += String(F("\r\nBaudRate          : ")) + String(Serial.baudRate());
	data += solidLine;
	String startup_file = *memo.GetValue("STARTUP_FILE");
	if (startup_file == "")startup_file = F("No-file"); else startup_file;
	data += String(F("\r\nStartup program   : ")) + startup_file;
	data += String(F("\r\nPublish time      : ")) + publish_Time;
	data += String(F("\r\nDeployed time     : ")) + dt;
	data += String(F("\r\nStartup time      : ")) + StartUp_datetime;
	data += String(F("\r\nCurrent time      : ")) + datetime;
	data += String(F("\r\nVersion           : ")) + String(FIRMWARE_VERSION);
	//////////////////////////////////////////////////////
	//data += String(F("\r\nFowarding Serial  : ")) + (Forwarding_Serial ? "Yes" : "No");
	//////////////////////////////////////////////////////
	data += String(F("\r\nPhone|About       : 0948378786|https://homeos.com.vn"));
	data += String(F("\r\nHelp&API          : https://support.homeos.vn"));
	//data += String(F("\r\nLine/Function     : ")) + __LINE__ +  __FUNCTION__;
	return data;
}

tm cvt_date(char const* date) {
	char s_month[5];
	int month, day, year;
	tm tmel;
	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

	sscanf(date, "%s %d %d", s_month, &day, &year);

	month = (strstr(month_names, s_month) - month_names) / 3 + 1;

	tmel.tm_hour = tmel.tm_min = tmel.tm_sec = 0; // This was working perfectly until 3am then broke until I added this.
	tmel.tm_mon = month;
	tmel.tm_mday = day;
	// year can be given as full four digit year or two digts (2010 or 10 for 2010);
	//it is converted to years since 1970
	if (year > 99)
		tmel.tm_year = year;
	else
		tmel.tm_year = year + 2000;

	return tmel;
}
uint8_t ICACHE_FLASH_ATTR GetRSSIState(int val) {
	if (val <= 0 && val >= -30) return 1;
	if (val < -30 && val >= -67) return 2;
	if (val < -67 && val >= -70) return 3;
	if (val < -70 && val >= -80) return 4;
	if (val < -80) return 5;
	return 0;
}
void Sleep(String& data) {
	if (data == "MAX")
		ESP.deepSleepMax();
	if (data.startsWith("D"))
	{
		data.remove(0, 1);
		ESP.deepSleepInstant(data.toInt());
	}
	else
		if (data == "")
			ESP.deepSleep(10e6);
		else
			ESP.deepSleep(data.toInt(), RF_CAL);
}