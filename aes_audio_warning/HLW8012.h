#ifndef __HLW8012_H_
#define __HLW8012_H_

#include <Wire.h>
#include "HelpString.h"
#include "RTC_DS3231.h"

#define VOLTAGE_THRESHOLD_UNDER			(215U)
#define VOLTAGE_THRESHOLD_UPPER			(225U)

#define HLW8012_CMD_RESET_ENERGY		(0x04U)

//Địa chỉ của HLW8012 giao tiếp qua I2C
#define HLW8012_ADDRESS					(0x08)
#define HLW8012_BYTE_REQUEST			(12U)
extern bool  HLW8012_isRealtime;

typedef struct
{
	float Voltage;
	float Current;
	float Power;
	float PowerFactor;
	float Energy;
	uint8_t Error;
	uint8_t Version;
} HLW8012_t;

extern HLW8012_t HLW8012;
extern void IRAM_ATTR HLW8012_Read();
extern String HLW8012_Get();
extern String HLW8012_ResetEnergy(String data);
extern String HLW8012_GetVersion(String data);

#endif	/* HLW8012_H_ */