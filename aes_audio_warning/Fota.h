#ifndef __FOTA_H_
#define __FOTA_H_

#include "Config.h"
#include <WString.h>

#if (0)
#define FotaDbg_Println(...)						do {Serial.println(__VA_ARGS__);} while(0);
#define FotaDbg_Printf(...)							do {Serial.printf(__VA_ARGS__);} while(0);
#define FotaDbg_Print(...)							do {Serial.print(__VA_ARGS__);} while(0);
#else
#define FotaDbg_Println(...)						do {} while(0)
#define FotaDbg_Printf(...)							do {} while(0)
#define FotaDbg_Print(...)							do {} while(0)
#endif
extern void Fota_Download(String type,String url, String user, String pass, String fName, uint16_t bufferSize, String* ret);
extern void Fota_Update(String filename);
#endif	/* __FOTA_H_ */