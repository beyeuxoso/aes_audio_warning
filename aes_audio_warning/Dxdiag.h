// Dxdiag.h

#ifndef _DXDIAG_h
#define _DXDIAG_h

#include "ESP8266WiFi.h"
#include "Config.h"

extern String publish_Time;

extern String GetTypeDevice(String data);
extern String ICACHE_FLASH_ATTR DXDIAG_Peripheral();
extern String ICACHE_FLASH_ATTR DXDIAG(String data);
tm cvt_date(char const* date);
uint8_t ICACHE_FLASH_ATTR GetRSSIState(int val);
void Sleep(String& data);
#endif

