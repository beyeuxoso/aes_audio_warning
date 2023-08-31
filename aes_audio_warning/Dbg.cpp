

#include "Dbg.h"


SoftwareSerial audio(A0, 2);
//SoftwareSerial dbg(5, 4);

void Dbg_Init()
{
	audio.begin(9600);
	//dbg.begin(DBG_BAUDRATE);
	Serial.begin(DBG_BAUDRATE);
	//pinMode(LED_DEBUG, OUTPUT);
}