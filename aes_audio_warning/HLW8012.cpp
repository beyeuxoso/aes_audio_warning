
#include "HLW8012.h"

bool HLW8012_isRealtime = true;

HLW8012_t HLW8012 = {};

void IRAM_ATTR HLW8012_Read()
{
	uint8_t data[HLW8012_BYTE_REQUEST];
	Wire.requestFrom(HLW8012_ADDRESS, HLW8012_BYTE_REQUEST);
	if (Wire.available())
	{
		for (int i = 0; i < HLW8012_BYTE_REQUEST; i++)
		{
			data[i] = Wire.read();
		}
		// Voltage
		HLW8012.Voltage = (float)((data[1] << 8) | (data[0])) / 100;
		// Current
		HLW8012.Current = (float)(data[2]) / 100;
		// Power Factor
		HLW8012.PowerFactor = (float)(data[3]) / 100;
		// Power
		HLW8012.Power = (float)((data[5] << 8) | data[4]) / 100;
		// Energy
		HLW8012.Energy = (double)((data[9] << 24) | (data[8] << 16) | (data[7] << 8) | (data[6])) / 1000;
		// Error
		HLW8012.Error = data[10];
		// Version
		HLW8012.Version = data[11];
	}
}
String HLW8012_Get()
{
	String data;
	data = "T:" + datetime + ", U:" + String(HLW8012.Voltage) + ", I:" + String(HLW8012.Current) + \
		   ", P:" + String(HLW8012.Power) + ", PF:" + String(HLW8012.PowerFactor) + ", E:" + String(HLW8012.Energy,3) + ", ER:" + String(HLW8012.Error);
	return data;
}

String HLW8012_ResetEnergy(String data)
{
	Wire.beginTransmission(HLW8012_ADDRESS);
	Wire.write(HLW8012_CMD_RESET_ENERGY);
	Wire.endTransmission();
	return "Reset energy done";
}

String HLW8012_GetVersion(String data)
{
	return String(uint8_t(HLW8012.Version / 100)) + "." + String((HLW8012.Version / 10) % 10) + "." + String(HLW8012.Version % 100);
}