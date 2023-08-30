
#include "GPIO.h"

//  Pinout ESP8266
//	RST					TX (Dbg)	1
//	A0	17	ADC			RX (Dbg)	3
//	D0	16				D1 (SCL)	5
//	D5	14	SCL			D2 (SDA)	4
//	D6	12	MISO		D3 (ISR)	0
//	D7	13	MOSI		D4 (ISR)	2
//	D8	15	CS			GND
//	3V3					5V

pinInfo_t pinTable[PIN_NUMBER_AVAILABLE] =
{
	{ "A0" , 17U, NOT_USED, 0xFF},
	{ "D0" , 16U, NOT_USED, 0xFF},
	{ "D3" ,  0U, NOT_USED, 0xFF},
	{ "D4" ,  2U, NOT_USED, 0xFF},
#if (!USING_SDCARD)
	{ "D5" , 14U, NOT_USED, 0xFF},
	{ "D6" , 12U, NOT_USED, 0xFF},
	{ "D7" , 13U, NOT_USED, 0xFF},
	{ "D8" , 15U, NOT_USED, 0xFF},
#endif
};
// Phải trả về giá trị chân pin dưới dạng số
bool checkValid(String pinName, uint8_t* pinNum, uint8_t pMode, uint8_t* pinIdx)
{
	bool checkFlag = false;
	for (uint8_t i = 0; i < PIN_NUMBER_AVAILABLE; i++)
	{
		if ((pinName == pinTable[i].Name) || (*pinNum == pinTable[i].Num))
		{
			*pinNum = pinTable[i].Num;
			if ((pinTable[i].Mode != pMode)&&(pMode != NOT_USED))
			{
				pinTable[i].Mode = pMode;
				pinMode(pinTable[i].Num, pMode);
			}
			*pinIdx = i;
			checkFlag = true;
			break;
		}
	}
	return checkFlag;
}

String GPIO_Process(String pinName, uint16_t* val, uint8_t pMode, procType_t procType)
{
	uint8_t pinNum = pinName.toInt();
	String ret;
	//FSPinInf_t PinInfo;
	uint8_t pinIdx;
	if (checkValid(pinName, &pinNum, pMode, &pinIdx))
	{
		// Procces GPIO
		switch (procType)
		{
			case DIGITAL_WRITE:
				digitalWrite(pinNum, (*val ? LOW : HIGH));
				ret = DONE;
				break;
			case DIGITAL_READ:
				*val = digitalRead(pinNum);
				ret = DONE;
				break;
			case ANALOG_WRITE:
				if (*val > 255) *val = 255;
				analogWrite(pinNum, *val);
				ret = DONE;
				break;
			case ANALOG_READ:
				*val = analogRead(pinNum);
				ret = DONE;
				break;
		}
	}
	else
	{
		ret = FAILED;
	}
	return ret;
}