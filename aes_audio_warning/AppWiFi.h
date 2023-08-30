#ifndef __APPLICATION_WIFI_H_
#define __APPLICATION_WIFI_H_

#include "Config.h"
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#define WIFI_MULTI_NUMMBER_MAX				(10U)
#define WIFI_MULTI_CONNECT_TIMEOUT			(1500U)
#define WIFI_MULTI_RETRY_CONNECT			(3U)

//static const char* ssid = "IIoT_Room_2.4Ghz";
//static const char* pass = "1234567890";

//const char* ssid = "S207_065A2G";
//const char* wifiPassword = "12345689";
typedef struct
{
	String ssid;
	String password;
} WiFiInf;

extern ESP8266WiFiMulti wifiMulti;
extern void AppWiFi_GetSSID();
extern void AppWiFi_ScanNetwork(dataType_t type, String* data);
extern void AppWiFi_Init();
extern void AppWiFi_Main();

#endif