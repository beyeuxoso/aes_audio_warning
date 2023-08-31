
#ifndef __DEBUG_H_
#define __DEBUG_H_

#include <Arduino.h>
#include <SoftwareSerial.h>

#define DBG_BAUDRATE	(115200U)
#define LED_DEBUG		(2U)
//#define dbg				(Serial)
#if (0)
#define Dbg_Println(...)						do {dbg.println(__VA_ARGS__);} while(0);
#define Dbg_Printf(...)							do {dbg.printf(__VA_ARGS__);} while(0);
#define Dbg_Print(...)							do {dbg.print(__VA_ARGS__);} while(0);
#else
#define Dbg_Println(...)						do {} while(0)
#define Dbg_Printf(...)							do {} while(0)
#define Dbg_Print(...)							do {} while(0)
#endif

extern SoftwareSerial audio;
//extern SoftwareSerial dbg;
extern void Dbg_Init();
#endif