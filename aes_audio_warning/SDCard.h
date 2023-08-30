
#ifndef __SD_CARD_H
#define __SD_CARD_H

#include <Arduino.h> 
#include <WString.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

//Địa chỉ cs của SPI dành cho thẻ nhớ
#define SD_CSPIN				(15U)
// Read file method
#define READ_FILE_STREAM				(1U)
#define STREAM_BUFFER_SIZE				(1024U)
enum
{
	CONNECTED,
	DISCONNECTED,
} SDCard_Status;

typedef enum
{
	SD_WIFI,
	SD_SCREEN,
	SD_Initialization,
	SD_DEVICE_NAME,
} CfgType;

typedef struct
{
	char* dataptr;
	uint32_t pos;
	uint16_t cnt;
	uint16_t size;
} StreamInf_t;

const String RootDir			= "Administrator/Default-Project/";
// Batchs
const String BatchDir			= "System/Batchs/";
const String WiFiDir			= "System/Batchs/CONFIG_WIFI.hbat";
const String InitializationDir	= "System/Batchs/Initialization.hbat";
// Screen
const String ScreenDir			= "System/Screens/";

const String DevNameDir			= "System/String/Str_DEVICE_NAME.hos";

//Firmware
const String FirmwareDir		= "System/Firmware/";
// Functions

// Screens

// Allocate

// Startup
extern void SDCard_Init();
extern bool SDCard_CreateDirectory(String dir);
extern bool SDCard_RemoveDirectory(String dir);

extern String SDCard_LoadCfg(CfgType type);
extern String SDCard_ReadFile(String fname);
extern void SDCard_SaveCfg();

extern String SDCard_LoadFile(String fileName);
uint16_t SDCard_ListFile(String dir, char* list, String exFile);
extern String SDCard_ListDir(File& dir, int numTabs);
extern bool SDCard_ReadFileStream(StreamInf_t* stream, String fileName);

extern void SDCard_WriteFile(String fname, String content, String endline);
extern bool SDCard_helpParseBatchFile(String dir, String* result, uint16_t timeout = 2500U);

#endif