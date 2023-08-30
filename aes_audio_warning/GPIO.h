
#ifndef __HLD_GPIO_H_
#define __HLD_GPIO_H_

#include <Arduino.h>
#include "Config.h"

//  Pinout ESP8266
//	RST					TX (Dbg)	1
//	A0		ADC			RX (Dbg)	3
//	D0	16				D1 (SCL)	5
//	D5	14	SCL			D2 (SDA)	4
//	D6	12	MISO		D3 (ISR)	0
//	D7	13	MOSI		D4 (ISR)	2
//	D8	15	CS			GND
//	3V3					5V
#define NOT_USED					(0xFF)

#if(!USING_SDCARD)
#define PIN_NUMBER_AVAILABLE		(8U)
#else
#define PIN_NUMBER_AVAILABLE		(4U)
#endif

typedef struct
{
	String Name;
	uint8_t Num;
	uint8_t Mode;
	uint8_t Value;
} pinInfo_t;

typedef enum
{
	DIGITAL_WRITE,
	DIGITAL_READ,
	ANALOG_WRITE,
	ANALOG_READ,
} procType_t;

extern pinInfo_t pinTable[PIN_NUMBER_AVAILABLE];
extern String GPIO_Process(String pinName, uint16_t* val, uint8_t pinType, procType_t pType);
#endif