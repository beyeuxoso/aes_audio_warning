#ifndef __DEVICE_CONFIGURATION_
#define __DEVICE_CONFIGURATION_

// Device information
#define FIRMWARE_VERSION				"[LGS]1.0.8"


// WiFi Feature
#define USING_WIFI_EXTENDER				(0U)
// IoT server
#define USING_WEBSOCKET_SERVER			(1U)

// Feature using
#define USING_SDCARD					(1U)
#define USING_EEPROM					(0U)
#define USING_HLW8012					(1U)

// Pin Interrupt
#define USING_PIN_INTERRUPT				(0U)

// Common define
#define NONE							"None"
#define DONE							"Done"
#define NOT_SUPPORT						"Not support"
#define FAILED							"Failed"

// ...
typedef enum
{
	JSON_DATA,
	NORMAL_DATA,
} dataType_t;
// ...
#define LEFT	0U
#define RIGHT	1U

#endif	/* __DEVICE_CONFIGURATION_ */