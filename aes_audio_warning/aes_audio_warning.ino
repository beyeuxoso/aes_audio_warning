/*
 Name:		aes_audio_warning.ino
 Created:	8/30/2023 10:54:18 PM
 Author:	Công Minh
*/

#include <Ticker.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <Wire.h>
#include <SD.h>
#include <SPI.h>
#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <ESP8266WiFi.h>

#include "Config.h"
#include "Threads.h"
#include "Dbg.h"
#include "AppWiFi.h"
#include "WSClient.h"
#include "WSHandle.h"
#include "SDCard.h"
#include "ntp.h"
#include "Watchdog.h"
#include "Memory.h"
#include "serial_handle.h"
#include "dfplayer.h"


static void Startup()
{
	String startup = memo.GetStartup(memo.CurrentUser, memo.CurrentProject);
	if (memo.ExistName("STARTUP_FILE")) {
		String* su = memo.GetValue("STARTUP_FILE");
		if (SD.exists(startup + *su + ".hos"))
			startup += *su + ".hos";
		else
			startup += "Startup.hos";
	}
	else
	{
		startup += "Startup.hos";
	}
	if (SD.exists(startup))
	{
		String ret;
		SDCard_helpParseBatchFile(startup, &ret);
		WSClient_PushFiFoBuffer(ret);
	}
};

// the setup function runs once when you press reset or power the board
void setup() {
	// Debug via Serial
	Dbg_Init();
	dfplayer_init();
	//Init I2C
	Wire.begin();
#if USING_SDCARD
	// Connect to SD card
	SDCard_Init();
#endif
	//
	Memory_Initial();
	//
	Preprocess_Init();
	// Start connect to WiFi
	AppWiFi_Init();
	//
	WSHandle_Init();
	// Start tasks which needed to using internet
	WSClient_Init();
	NTP_Init();
	// Cài đặt thời gian chạy các thread
	Threads_Init();
	// Run startup file
	Startup();
	// Startup for Smartlight
//#if (MAIN_PROJECT == SMARTLIGHT_NEMA_PRJ)
//	String state = *memo.GetValue("STATE_LED"); state.trim();
//	int _state = (state.toInt() >= 1) ? 0 : 1;
//	String _dim = *memo.GetValue("DIMM_LED"); _dim.trim();
//	WSClient_PushFiFoBuffer("DIGITAL WRITE 0 " + String(_state) + ";");
//	WSClient_PushFiFoBuffer("ANALOG WRITE 16 " + _dim + ";");
//#endif
	// Enable Watchdog
	Wdg_Enable(TIMMER_CNT_6000MS);
	//
	SoftWdg_Enable(TIMMER_CNT_150S);

}

// the loop function runs over and over again until power down or reset
void loop() {
	// Run Threads
	Threads_Run();
#if USING_WEBSOCKET_SERVER
	if (WiFi.isConnected() == true)
	{
		WSClient_Main();
	}
	// Handle message received for websocket client
	WSHandle_Main();
#endif
	Uart_Event();
	UartHandle_Main();
	// Reset Hardware Watchdog Timmer
	Wdg_Feed();
	// Reset Software Watchdog Timmer
	SoftWdg_Feed();
}

