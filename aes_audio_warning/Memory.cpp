
#include "Memory.h"

void MemoryClass::initial(size_t size)
{
	LastAllocateData = 0;
	LastAddress = 0;
	for (uint8_t i = 0; i < MAX_VARIABLE; i++)
	{
		ListAllocateAddress[i].Name = "";
	}
}

void MemoryClass::initial()
{
	LastAllocateData = 0;
	LastAddress = 0;
	LoadMemory();
}
void MemoryClass::ClearStructValue(uint index) {
	ListAllocateAddress[index].Page = 0; //Trang nhớ
	ListAllocateAddress[index].Address = 0; // Địa chỉ vùng nhớ thực tế
	ListAllocateAddress[index].Name = ""; //Tên biến
	ListAllocateAddress[index].Type = MemoryType::VAR; // Kiểu dữ liệu
	ListAllocateAddress[index].isNegative = false; //Nếu là kiểu số, thì là số âm hay dương, mặc định là dương => false
	ListAllocateAddress[index].Value = ""; // Giá trị lưu trữ
	ListAllocateAddress[index].Size = 8; // Giá trị lưu trữ
	ListAllocateAddress[index].isRam = false; // nếu biến khai báo chỉ tạo trên RAM, khi khởi động lại thì xoá, ngược lại thì lưu trữ vào flash
}
void  MemoryClass::ResetAllocateAddress() {
	AllocateOnFlash = "";
	ValueOnFlash = "";
	LastAllocateData = 0;
	LastAddress = 0; 
}
void MemoryClass::ClearAllMemory() {
	if (_onClearMemory != NULL && USING_SD)
		_onClearMemory(); 
}
TcpClient MemoryClass::GetTcpClientByName(String name)
{
	for (uint8_t i = 0; i < ListTcpClients.size(); i++)
	{
		if (ListTcpClients.get(i).Name == name) return ListTcpClients.get(i);
	}
	return TcpClient();
}
void MemoryClass::LoadMemory()
{
	if (_onLoadMemory != NULL && USING_SD)
	{
		_onLoadMemory();
	}
	SerializeAllocate();
	delay(10);
}

String MemoryClass::GetMemoryDataCurrent()
{
	return ValueOnFlash;
}

String MemoryClass::SetMemoryDataCurrent(String data)
{
	ValueOnFlash = data;
	return ValueOnFlash;
}

String MemoryClass::GetAllocateMemoryDataCurrent()
{
	return AllocateOnFlash;
}

String MemoryClass::SetAllocateMemoryDataCurrent(String data)
{
	AllocateOnFlash = data;
	return AllocateOnFlash;
}

#pragma region Size and Type 
uint MemoryClass::GetSizeVariableType(MemoryType type)
{
	MemorySize ms;
	switch (type)
	{
	case MemoryType::BOOL: return ms.BOOL;
	case MemoryType::BYTE: return ms.BYTE;
	case MemoryType::INT: return ms.INT;
	case MemoryType::NUMBER: return ms.NUMBER;
	case MemoryType::DATE: return ms.DATE;
	case MemoryType::DATETIME: return ms.DATETIME;
	case MemoryType::TIME: return ms.TIME;
	case MemoryType::VAR: return ms.VAR;
	case MemoryType::TCPCLIENT: return ms.TCPCLIENT;
	case MemoryType::SOCKETCLIENT: return ms.SOCKETCLIENT;
	case MemoryType::FTPCLIENT: return ms.FTPCLIENT;
	case MemoryType::WIFI: return ms.WIFI;
	case MemoryType::FUNCTION: return ms.FUNCTION;
	case MemoryType::TABLE: return ms.TABLE;
	case MemoryType::DATABASE: return ms.DATABASE;
	default: return 8;
	}
}
uint MemoryClass::GetSizeVariableAddress(uint8_t add)
{
	MemorySize ms;
	switch (add)
	{
	case 1: return ms.BOOL;
	case 2: return ms.BYTE;
	case 3: return ms.INT;
	case 4: return ms.DATE;
	case 5: return ms.DATETIME;
	case 6: return ms.TIME;
	case 7: return ms.VAR;
	case 8: return ms.TCPCLIENT;
	case 9: return ms.FTPCLIENT;
	case 10: return ms.WIFI;
	case 11: return ms.FUNCTION;
	case 12: return ms.NUMBER;
	case 13: return ms.TABLE;
	case 14: return ms.DATABASE;
	case 15: return ms.SOCKETCLIENT;

	default: return 8;
	}
}
uint MemoryClass::GetAddressByType(MemoryType type)
{
	MemorySize ms;
	switch (type)
	{
	case MemoryType::BOOL: return 1;
	case MemoryType::BYTE: return 2;
	case MemoryType::INT: return 3;
	case MemoryType::DATE: return 4;
	case MemoryType::DATETIME: return 5;
	case MemoryType::TIME: return 6;
	case MemoryType::VAR: return 7;
	case MemoryType::TCPCLIENT: return 8;
	case MemoryType::FTPCLIENT: return 9;
	case MemoryType::WIFI: return 10;
	case MemoryType::FUNCTION: return 11;
	case MemoryType::NUMBER: return 12;
	case MemoryType::TABLE: return 13;
	case MemoryType::DATABASE: return 14;
	case MemoryType::SOCKETCLIENT: return 15;
	default: return 0;
	}
}
MemoryType MemoryClass::GetTypeByAddress(uint add)
{
	MemorySize ms;
	switch (add)
	{
	case 1: return MemoryType::BOOL;
	case 2: return MemoryType::BYTE;
	case 3: return MemoryType::INT;
	case 4: return MemoryType::DATE;
	case 5: return MemoryType::DATETIME;
	case 6: return MemoryType::TIME;
	case 7: return MemoryType::VAR;
	case 8: return MemoryType::TCPCLIENT;
	case 9: return MemoryType::FTPCLIENT;
	case 10: return MemoryType::WIFI;
	case 11: return MemoryType::FUNCTION;
	case 12: return MemoryType::NUMBER;
	case 13: return MemoryType::TABLE;
	case 14: return MemoryType::DATABASE;
	case 15: return MemoryType::SOCKETCLIENT;
	default: return MemoryType::VAR;
	}
}
#pragma endregion

TcpClient  MemoryClass::CreateTcp(String data, String type) {
	TcpClient tc;
	String name = data.subString(0, data.indexOf("="));
	data.remove(0, name.length() + 1);
	tc.Name = name;
	tc.Type = type;
	tc.Host = data.subString(0, data.indexOf(":"));
	tc.Port = (data.substring(data.indexOf(":"))).toInt();
	//tc.Timeout = (data.substring(data.indexOf(":"))).toInt();
	for (uint8_t i = 0; i < ListTcpClients.size(); i++)
	{
		if (ListTcpClients.get(i).Name == "") {
			tc.Index = i;
			break;
		}
	}
	if (type == "TCP") Declare("TCPCLIENT " + name);
	if (type == "UDP") Declare("UDPCLIENT " + name);
	if (type == "FTP") Declare("FTPCLIENT " + name);
	SetValue(name, data);
	SaveMemory();
	//Debug("Created " + type + " client with host/port[" + String(tc.Index) + "]: " + tc.Host + ":" + String(tc.Port), true);
	return tc;
}
bool MemoryClass::Declare(String data) {
	return CommandCreateVar(data, false);
}
bool MemoryClass::Let(String data) {
	return CommandCreateVar(data, true);
}
String MemoryClass::DefaultValue(MemoryType type) {
	MemorySize ms;
	switch (type)
	{
	case MemoryType::BOOL: return "1";
	case MemoryType::BYTE:
	case MemoryType::INT:
	case MemoryType::NUMBER:  return "0";
	case MemoryType::VAR:  return "";
	case MemoryType::DATE: return "2000-01-01";
	case MemoryType::DATETIME: return "2000-01-01 00:00:00";
	case MemoryType::TIME: return "00:00:00";
	default: return "";
	}
}
bool MemoryClass::Set(String data) {
	String prop = data.subString(0, data.indexOf("="));
	data.remove(0, data.indexOf("=") + 1);
	data.trim();
	String value;
	bool hasChar = false;
	if (data.indexOf("'") >= 0 && data.lastIndexOf("'") >= 0) {
		hasChar = true;
		value = data.subString(data.indexOf("'") + 1, data.lastIndexOf("'") - 1);
	}
	else
		value = data;
	if (!hasChar && value != "" && ExistName(value)) {
		value = *GetValue(value);
	}
	if (!hasChar && value != "" && !ExistName(value)) {
		//Debug("Assign expression: " + data);
		if (_onExpression != NULL) {

			//Debug("here:" + prop + ", value:" + value, true);
			value = _onExpression(value);
		}
		else
		{
			//Debug("no assign:" + prop + ", value:" + value, true);
		}
	}
	//Debug("prop:" + prop + ", value:" + value, true);
	if (value == "NULL" || value == "EMPTY") value = "";
	AllocateAddress aa = GetPropertiesByName(prop);
	if (aa.Type == MemoryType::INT ||
		aa.Type == MemoryType::BYTE ||
		aa.Type == MemoryType::BOOL) value = String(atof(value.c_str()), 0);
	if (aa.Type == MemoryType::NUMBER) {
		uint sp = 0;
		if (aa.DecimalPlace != "")   sp = aa.DecimalPlace.split(",", 1).toInt();
		value = String(atof(value.c_str()), sp);
	}
	SetValue(prop, value);
	return true;
}
String MemoryClass::FormatValue(String name, double value) {
	AllocateAddress aa = GetPropertiesByName(name);
	switch (aa.Type)
	{
	case MemoryType::INT: return String(value, 0); break;
	default:
		break;
	}
	return String(value);
}
String* MemoryClass::Get(String data) {
	data.trim();
	return GetValue(data);
}
bool MemoryClass::CommandCreateVar(String data, bool isram) {
	String nm;
	String val = "";
	if (data.startsWith("INT ")) {
		data.remove(0, String("INT ").length());
		data.trim();
		val = "0";
		data.replace(" ", "");
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		//Debug("CREATE INT " + nm + ",is ram: " + String(isram) + ",value: " + val, false);
		bool v = CreateVariable(nm, MemoryType::INT, "5", val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}
	if (data.startsWith("TCPCLIENT ")) {
		data.remove(0, String("TCPCLIENT ").length());
		data.trim();
		nm = data.subString(0, data.indexOf("="));
		data.remove(0, data.indexOf("=") + 1);
		val = data;
		bool v = CreateVariable(nm, MemoryType::TCPCLIENT, String(GetSizeVariableType(MemoryType::TCPCLIENT)), val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}
	if (data.startsWith("SOCKETCLIENT ")) {
		data.remove(0, String("SOCKETCLIENT ").length());
		data.trim();
		nm = data.subString(0, data.indexOf("="));
		data.remove(0, data.indexOf("=") + 1);
		val = data;
		bool v = CreateVariable(nm, MemoryType::SOCKETCLIENT, String(GetSizeVariableType(MemoryType::SOCKETCLIENT)), val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}

	if (data.startsWith("BYTE ")) {
		data.remove(0, String("BYTE ").length());
		data.trim();
		val = "0";
		data.replace(" ", "");
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		//Debug("CREATE BYTE " + nm, false);
		bool v = CreateVariable(nm, MemoryType::BYTE, "3", val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}
	if (data.startsWith("BOOL ")) {
		data.remove(0, String("BOOL ").length());
		data.trim();
		val = "1";
		data.replace(" ", "");
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		//Debug("CREATE BOOL " + nm, false);
		bool v = CreateVariable(nm, MemoryType::BOOL, "1", val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}
	if (data.startsWith("DATE ")) {
		data.remove(0, String("DATE ").length());
		data.trim();
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		if (val.indexOf("'") >= 0) {
			val.replace("'", "");
		}
		bool v = CreateVariable(nm, MemoryType::DATE, String(GetSizeVariableType(MemoryType::DATE)), val, isram);
		/*	if (val != "" && v)
			{

				SetValue(nm, val);
			}*/
		return true;
	}
	if (data.startsWith("TIME ")) {
		data.remove(0, String("TIME ").length());
		data.trim();
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		if (val.indexOf("'") >= 0) {
			val.replace("'", "");
		}
		bool v = CreateVariable(nm, MemoryType::TIME, String(GetSizeVariableType(MemoryType::TIME)), val, isram);
		//if (val != "" && v)
		//{
		//
		//	SetValue(nm, val);
		//}
		return true;
	}
	if (data.startsWith("DATETIME ")) {
		data.remove(0, String("DATETIME ").length());
		data.trim();
		if (data.indexOf("=") >= 0) {
			nm = data.split("=", 0);
			val = data.split("=", 1);
			nm.trim();
			val.trim();
		}
		else
			nm = data;
		if (val.indexOf("'") >= 0) {
			val.replace("'", "");
		}
		bool v = CreateVariable(data, MemoryType::DATETIME, String(GetSizeVariableType(MemoryType::DATETIME)), val, isram);
		//if (val != "" && v)
		//{
		//
		//	SetValue(nm, val);
		//}
		//Debug(val + "," + nm, false);
		return true;
	}
	if (data.startsWith("WIFI ")) {
		data.remove(0, String("WIFI ").length());
		data.trim();
		nm = data.subString(0, data.indexOf("="));
		data.remove(0, data.indexOf("=") + 1);
		val = data;
		bool v = CreateVariable(nm, MemoryType::WIFI, String(GetSizeVariableType(MemoryType::WIFI)), val, isram);

		/*	if (val != "" && v)
			{
				SetValue(nm, val);
			}*/
		return true;
	}
	if (data.startsWith("FUNCTION ")) {
		data.remove(0, String("FUNCTION ").length());
		data.trim();
		//String nm = data;//.subString(0, data.indexOf("="));
		//String val = data.substring(data.indexOf("=") + 1);
		bool v = CreateVariable(data, MemoryType::FUNCTION, String(GetSizeVariableType(MemoryType::FUNCTION)), "1", isram);
		//if (v)
		//{
		//	SetValue(nm, "1");
		//}
		return true;
	}
	if (data.startsWith("TABLE ")) {
		data.remove(0, String("TABLE ").length());
		data.trim();
		bool v = CreateVariable(data, MemoryType::TABLE, String(GetSizeVariableType(MemoryType::TABLE)), "T", isram);
		return true;
	}
	if (data.startsWith("DATABASE ")) {
		data.remove(0, String("DATABASE ").length());
		data.trim();
		bool v = CreateVariable(data, MemoryType::DATABASE, String(GetSizeVariableType(MemoryType::DATABASE)), "D", isram);
		return true;
	}
	if (data.startsWith("FTPCLIENT ")) {
		data.remove(0, String("FTPCLIENT ").length());
		data.trim();
		nm = data.subString(0, data.indexOf("="));
		data.remove(0, data.indexOf("=") + 1);
		val = data;
		bool v = CreateVariable(nm, MemoryType::FTPCLIENT, String(GetSizeVariableType(MemoryType::FTPCLIENT)), val, isram);
		//if (val != "" && v) SetValue(nm, val);
		return true;
	}
	if (data.startsWith("CHAR[")) { //VAR(10) SoA = 'hello world' 
		data.remove(0, String("CHAR[").length());
		data.trim();//10 SoA = 'hello world' 
		String valX = data.subString(0, data.indexOf("]"));
		data.remove(0, data.indexOf(" "));
		data.trim();//SoA = 'hello world'
		if (data.indexOf("=") >= 0) {
			nm = data.subString(0, data.indexOf("="));
			data.remove(0, data.indexOf("=") + 1);
			data.trim();//'hello world'
			if (data.indexOf("'") >= 0 && data.lastIndexOf("'") >= 0)
			{
				uint8_t s = data.indexOf("'"), e = data.lastIndexOf("'");
				val = data.subString(s + 1, e - 1);
			}
			else val = data;
			//Debug("Combine variable:" + val);
			val.trim();
		}
		else
			nm = data;
		nm.trim();
		bool v = CreateVariable(nm, MemoryType::VAR, "1", val, isram);
		//Debug("Created variable:" + nm + ", with value:" + val);

		//if (val != "" && v)
		//{
		//	SetValue(nm, "1");
		//}
		return true;
	}
	if (data.startsWith("NUMBER[")) { //VAR(10) SoA = 'hello world'
		data.remove(0, String("NUMBER[").length());
		data.trim();//10,1 SoA = 'hello world'
		String strX = data.subString(0, data.indexOf("]"));
		data.remove(0, data.indexOf(" "));
		data.trim();//SoA = 'hello world'
		if (data.indexOf("=") >= 0) {
			nm = data.subString(0, data.indexOf("="));
			data.remove(0, data.indexOf("=") + 1);
			data.trim();//'hello world'
			val = data;
		}
		else
			nm = data;
		nm.trim();
		bool v = CreateVariable(nm, MemoryType::NUMBER, strX, val, isram);
		//if (val != "" && v)
		//{
		//	SetValue(nm, val);
		//}
		return true;
	}
	//Debug("unknown this [" + data + "]", true);
	return true;
}

void MemoryClass::onSaveToSD(void(*callback)(String name, String value, MemoryType memo_type))
{
	_onSaveToSD = callback;
}
void MemoryClass::onGetValueFromSD(String(*callback)(String name))
{
	_onGetValueFromSD = *callback;
}
void MemoryClass::onGetValue(String(*callback)(AllocateAddress aa))
{
	_onGetValue = *callback;
}
bool MemoryClass::CreateVariable(String name, MemoryType type, String size, String value, bool isram = false)//Done
{
	//Debug("Check Exist name:" + name + ", size: " + size + ",is RAM: " + String(isram), false);
	if (ExistName(name)) {
		return false;
	}
	LastAllocateData = AvailableAllocateAddress();
	if (LastAllocateData > MAX_VARIABLE) return false;
	name.trim();
	LastAddress = GetLastAddress();
	ListAllocateAddress[LastAllocateData].Name = name;
	ListAllocateAddress[LastAllocateData].Type = type;
	ListAllocateAddress[LastAllocateData].isNegative = false;
	ListAllocateAddress[LastAllocateData].isRam = isram;
	ListAllocateAddress[LastAllocateData].Page = 0;
	if (size.indexOf(",") >= 0) {
		ListAllocateAddress[LastAllocateData].Size = size.split(",", 0).toInt() + size.split(",", 1).toInt() + 1;
		ListAllocateAddress[LastAllocateData].DecimalPlace = size;
	}
	else {
		if (size.toInt() == 0) size = String(GetSizeVariableType(type));
		ListAllocateAddress[LastAllocateData].Size = size.toInt();
	}
	ListAllocateAddress[LastAllocateData].Value = (value == "") ? MemoryClass::DefaultValue(type) : value;
	ListAllocateAddress[LastAllocateData].Address = LastAddress;
	SetValue(name, value);
	//Debug("MemoryClass::CreateVariable->Name:" + ListAllocateAddress[LastAllocateData].Name + ",Address:" + String(LastAddress)
	//	+ ",Type:" + String(GetAddressByType(type)) + ",Default:" + ListAllocateAddress[LastAllocateData].Value +
	//	",Size:" + size + ", index: " + String(LastAllocateData), true);
	return true;
}
bool MemoryClass::isNumberic(String name) {
	bool res = true;
	for (uint8_t i = 0; i < name.length(); i++)
	{
		if (name[i] == '0' || name[i] == '1' || name[i] == '2' || name[i] == '3' || name[i] == '4' || name[i] == '5' ||
			name[i] == '6' || name[i] == '7' || name[i] == '8' || name[i] == '9' || name[i] == '-' || name[i] == '.') {
			res = true;
		}
		else
		{
			res = false;
			break;
		}
	}
	return res;
}
AllocateAddress MemoryClass::GetPropertiesByName(String name) {
	AllocateAddress aa;
	//Debug("finding... " + name);
	if (isNumberic(name)) return aa;
	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		if (ListAllocateAddress[i].Name == name) return ListAllocateAddress[i];
	}
	//Debug("Not found " + name);
	return aa;
}

bool MemoryClass::Dispose(String name)
{
	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		ListAllocateAddress[i].Name.trim();
		if (ListAllocateAddress[i].Name == name)
		{
			//Debug("Dispose Variable: " + name + ",State: Xoa rong ten bien", true);
			ListAllocateAddress[i].Name = "";
			break;
		}
	}
	SaveMemory();
	//Debug("Dispose Variable: " + name + ",State: Xoa rong ten bien", true);
	//ReOrganization();
	return true;
}
uint MemoryClass::AvailableAllocateAddress() {
	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		//Debug("before: " + ListAllocateAddress[i].Name,false);
		ListAllocateAddress[i].Name.trim();
		//Debug("after: " + ListAllocateAddress[i].Name, false);
		if (ListAllocateAddress[i].Name == "" || ListAllocateAddress[i].Name == "null") return i;
	}
	return MAX_VARIABLE + 1;
}
bool MemoryClass::ExistName(String name)
{
	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		if (ListAllocateAddress[i].Name == "") return false;
		if (ListAllocateAddress[i].Name == name) return true;
	}
	return false;
}
#pragma region format data
uint MemoryClass::toInt(String name) {
	return	GetPropertiesByName(name).Value.toInt();
}
float MemoryClass::toFloat(String name) {
	return	GetPropertiesByName(name).Value.toFloat();
}
double MemoryClass::toDou(String name) {
	return	GetPropertiesByName(name).Value.toDouble();
}
String MemoryClass::toYear(String name) {
	return	GetPropertiesByName(name).Value.subString(0, 4);
}
String MemoryClass::toMonth(String name) {//yyyy-mm-dd
	return	GetPropertiesByName(name).Value.subString(5, 2);
}
String MemoryClass::toDay(String name) {//yyyy-mm-dd
	return	GetPropertiesByName(name).Value.subString(8, 2);
}
String MemoryClass::toHour(String name) {//yyyy-mm-dd hh:mm:ss
	AllocateAddress d = GetPropertiesByName(name);
	//if (d.Type == MemoryType::DATETIME) 
	return	d.Value.subString(11, 2);
	//if (d.Type == MemoryType::TIME) return	d.Value.subString(0, 2);
	return "00";
}
String MemoryClass::toMinute(String name) {//yyyy-mm-dd hh:mm:ss
	AllocateAddress d = GetPropertiesByName(name);
	//if (d.Type == MemoryType::DATETIME) 
	return	d.Value.subString(14, 2);
	//if (d.Type == MemoryType::TIME) return	d.Value.subString(3, 2);
	return "00";
}
String MemoryClass::toSecond(String name) {//yyyy-mm-dd hh:mm:ss
	AllocateAddress d = GetPropertiesByName(name);
	//if (d.Type == MemoryType::DATETIME)
	return	d.Value.subString(17, 2);
	//if (d.Type == MemoryType::TIME) return	d.Value.subString(6, 2);
	return "00";
}
#pragma endregion

#pragma region Get,Set data
String TempValue = "";
AllocateAddress aa;
String* MemoryClass::GetValue(String name)
{
	aa = GetPropertiesByName(name);
	if (_onGetValueFromSD != NULL)
	{
		//Debug("GetValue found " + name);
		if (aa.Type == MemoryType::VAR || aa.Type == MemoryType::FUNCTION)
		{
			TempValue = _onGetValueFromSD(name);
			return &TempValue;
		}
	}
	if (_onGetValue != NULL)
	{
		TempValue = _onGetValue(aa);
		return &TempValue;
	}
	return	&aa.Value;
}

bool MemoryClass::SetValue(String name, String value)
{
	//Debug("befor->setValue: " + name + ",value:" + value, false);

	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		if (ListAllocateAddress[i].Name == "") break;
		if (ListAllocateAddress[i].Name == name) {
			//Debug("inside->setValue: " + name + ",value:" + value, false);
			if (value != "") value.trim();
			else
			{
				if (ListAllocateAddress[i].Type == MemoryType::VAR ||
					ListAllocateAddress[i].Type == MemoryType::WIFI
					|| ListAllocateAddress[i].Type == MemoryType::TCPCLIENT || ListAllocateAddress[i].Type == MemoryType::SOCKETCLIENT)
					value = value.padleftString(ListAllocateAddress[i].Size, " ");
				if (
					ListAllocateAddress[i].Type == MemoryType::NUMBER ||
					ListAllocateAddress[i].Type == MemoryType::INT ||
					ListAllocateAddress[i].Type == MemoryType::BYTE ||
					ListAllocateAddress[i].Type == MemoryType::BOOL
					) value = "0";
				if (ListAllocateAddress[i].Type == MemoryType::DATE) value = "1988-05-11";
				if (ListAllocateAddress[i].Type == MemoryType::DATETIME) value = "1988-05-11 18:45:00";
				if (ListAllocateAddress[i].Type == MemoryType::TIME) value = "18:45:00";
			}
			//Debug("1->setValue: " + name + ",value:" + value, false);

			if (ListAllocateAddress[i].Type == MemoryType::VAR ||
				ListAllocateAddress[i].Type == MemoryType::WIFI
				|| ListAllocateAddress[i].Type == MemoryType::TCPCLIENT || ListAllocateAddress[i].Type == MemoryType::SOCKETCLIENT || ListAllocateAddress[i].Type == MemoryType::FUNCTION) {
				if (value.length() > ListAllocateAddress[i].Size)
				{
					if (ListAllocateAddress[i].Type == MemoryType::VAR) {
						ListAllocateAddress[i].Value = "1";
						if (_onSaveToSD != NULL) _onSaveToSD(ListAllocateAddress[i].Name, value, ListAllocateAddress[i].Type);
					}
					else {
						ListAllocateAddress[i].Value = value.subString(0, ListAllocateAddress[i].Size);
						//Debug("Truncate data[" + ListAllocateAddress[i].Name + "], max size this one is " + String(ListAllocateAddress[i].Size) + " and this one is " + String(value.length()), true);
					}
				}
				else
				{
					ListAllocateAddress[i].Value = value;
				}
			}
			else
			{
				if (ListAllocateAddress[i].DecimalPlace != "") {
					value = String(atof(value.c_str()), ListAllocateAddress[i].DecimalPlace.split(",", 1).toInt());
					ListAllocateAddress[i].Value = value;
					//Debug("2->setValue: " + name + ",value:" + value, false);
				}
				else {
					value = String(atof(value.c_str()), 0);
					//Debug("2->setValue: " + name + ",value:" + value, false);
					ListAllocateAddress[i].Value = value;
				}
			}
			/*	if (name != "ADC_RAW" && name != "VCC_DATA")
					Debug("setValue: " + name + ",value:" + ListAllocateAddress[i].Value, false);*/
			return true;
		}
	}
	//if (name != "ADC_RAW" && name != "VCC_DATA")
	//	Debug("break setValue: " + name + ",value:" + value, false);
	return false;
}
bool MemoryClass::ClearValue(String name)
{
	return SetValue(name, "");
}
#pragma endregion

uint16_t MemoryClass::LastAllocate()
{
	return LastAllocateData;
}
uint MemoryClass::GetLastAddress()
{
	uint last = 0;
	for (uint i = 0; i < MAX_VARIABLE; i++)
	{
		ListAllocateAddress[i].Name.trim();
		if (ListAllocateAddress[i].Name != "") {

			last = ListAllocateAddress[i].Address + ListAllocateAddress[i].Size;
		}
	}
	return last;
}
uint16_t MemoryClass::length()
{
	return ValueOnFlash.length();
}
#pragma region Convert var, string

String MemoryClass::GetVariable(uint i) {
	String tempView = "";
	if (ListAllocateAddress[i].Name != "" && ListAllocateAddress[i].Name != "null") {
		String val = "NIL";
		if (ListAllocateAddress[i].Value != "")val = ListAllocateAddress[i].Value;
		tempView = ("\r\n#[" + String(i).padleft(3, "0") + "]:" +
			ListAllocateAddress[i].Name.padleftString(16, " ") +
			String(ListAllocateAddress[i].Address).padleft(6, " ") +
			String(GetAddressByType(ListAllocateAddress[i].Type)).padleft(5, " ") + "   " +
			String(ListAllocateAddress[i].Size).padleftString(5, " ") + "   " +
			val);
	}
	return tempView;
}
String MemoryClass::SerializeAllocate() {
	DynamicJsonDocument doc = ReadFileJSON(GetAllocateFileName(CurrentUser, CurrentProject));
	//serializeJson(doc, Serial);
	//Debug("User: " + CurrentUser + ", Project: " + CurrentProject + ", Size: " + String(doc.size()));
	if (doc.isNull())return "";
	if (doc.size() <= 0) return "";
	uint8_t count = 0;
	for (uint8_t i = 0; i < doc.size(); i++)
	{
		String name = doc[i]["N"].as<String>();
		if (name != "" && name != "null") {
			ListAllocateAddress[i].Address = 0;// doc[i]["ADDRESS"];
			ListAllocateAddress[i].Name = doc[i]["N"].as<String>();
			if (ListAllocateAddress[i].Name == "null") ListAllocateAddress[i].Name = "";
			ListAllocateAddress[i].Type = GetTypeByAddress(doc[i]["T"]);
			ListAllocateAddress[i].isNegative = false;// doc["IS_NEGATIVE"];
			ListAllocateAddress[i].isRam = false;// doc[i]["IS_RAM"];
			ListAllocateAddress[i].Page = doc[i]["P"];
			ListAllocateAddress[i].Size = doc[i]["S"];
			ListAllocateAddress[i].DecimalPlace = doc[i]["D"].as<String>();
			if (ListAllocateAddress[i].DecimalPlace == "null")  ListAllocateAddress[i].DecimalPlace = "";
			String val = doc[i]["V"].as<String>();
			//Debug("SerializeAllocate->Done:" + val + "n name: "+ name, false);
			ListAllocateAddress[i].Value = val;

			if (val == "null")  ListAllocateAddress[i].Value = "";
			if (ListAllocateAddress[i].Type == MemoryType::TCPCLIENT) {
				TcpClient tc;
				tc.Index = count++;
				tc.Name = name;
				tc.Type = "TCP";
				tc.Host = val.split(":", 0);
				tc.Port = val.split(":", 1).toInt();
				ListTcpClients.add(tc);
			}
		}
		else
		{
			//Debug("Ket thuc:"+String(doc.size()));
			break;
		}
	}
	return "";
	//Debug("SerializeAllocate->Done", true);
}
DynamicJsonDocument MemoryClass::ReadFileJSON(String fileName) {
	DynamicJsonDocument doc(4096);
	if (USING_SD) {
		if (!SD.exists(fileName))return doc;
		File root = SD.open(fileName, sdfat::O_READ);//FILE_READ sdfat::O_READ
		if (root) {
			deserializeJson(doc, root);
			root.close();
		}
	}
	else
	{
		deserializeJson(doc, fileName);
	}
	return doc;
}
DynamicJsonDocument MemoryClass::DeserializeAllocate() {
	// allocate the memory for the document
	DynamicJsonDocument doc(4096);
	//Debug("DynamicJsonDocument", false); 
	JsonArray arrayX = doc.to<JsonArray>();
	JsonObject row;// = array.createNestedObject();
	//Debug("row", false);
	for (uint8_t i = 0; i < MAX_VARIABLE; i++)
	{
		if (ListAllocateAddress[i].Name != "" && ListAllocateAddress[i].Name != "null" && !ListAllocateAddress[i].isRam) {
			JsonObject row = arrayX.createNestedObject();
			//row["ADDRESS"] = ListAllocateAddress[i].Address;
			row["N"] = ListAllocateAddress[i].Name;
			row["T"] = GetAddressByType(ListAllocateAddress[i].Type);
			//row["IS_NEGATIVE"] = ListAllocateAddress[i].isNegative;
			//row["IS_RAM"] = ListAllocateAddress[i].isRam;
			row["P"] = ListAllocateAddress[i].Page;
			row["S"] = ListAllocateAddress[i].Size;
			row["D"] = ListAllocateAddress[i].DecimalPlace;
			row["V"] = ListAllocateAddress[i].Value;
			//array.add(row);
			//serializeJson(array, Serial);
			//Debug("row " + String(i) + ", name:" + ListAllocateAddress[i].Name, false);
		}
	}
	//doc.add(array);
	//serializeJson(doc, Serial);
	return doc;
	//serializeJson(doc, AllocateOnFlash);
}
#pragma endregion

void MemoryClass::SaveMemory()
{
	DynamicJsonDocument djd = DeserializeAllocate();
	if (djd.capacity() > 0) 
	{
		if (USING_SD)
		{
			if (_onSaveMemory != NULL)_onSaveMemory(djd);

			if (SD.exists(GetAllocateFileName(CurrentUser, CurrentProject)))
			{
				SD.remove(GetAllocateFileName(CurrentUser, CurrentProject));
			}
			File root = SD.open(GetAllocateFileName(CurrentUser, CurrentProject), FILE_WRITE);
			if (root) {
				serializeJson(djd, root);
			}
			root.close();
			if (_onEvent_Saved != NULL && USING_SD) _onEvent_Saved();
		}
	}
}

void MemoryClass::onListen(void(*callback)())
{
	_onListen = callback;
}
void MemoryClass::onExpression(String(*callback)(String data))
{
	_onExpression = callback;
}
void MemoryClass::onLoadMemory(void(*callback)())
{
	_onLoadMemory = callback;
}
void MemoryClass::onSaveMemory(void(*callback)(DynamicJsonDocument djd))
{
	_onSaveMemory = callback;
}
void MemoryClass::onClearMemory(void(*callback)())
{
	_onClearMemory = callback;
}
void MemoryClass::onEvent_Saved(void(*callback)())
{
	_onEvent_Saved = callback;
}
void MemoryClass::onEvent_DataProcessed(void(*callback)(String command, String data, String typeSending, String format))
{
	_onEvent_DataProcessed = callback;
}

String MemoryClass::GetStartup(String user, String project) {
	String temp = StartupFile;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetLoop(String user, String project) {
	String temp = LoopFile;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetDirectoryFunction(String user, String project) {
	String temp = dirFunction;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetDirectoryLogger(String user, String project) {
	String temp = dirLogger;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetDirectoryString(String user, String project) {
	String temp = dirString;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetDirectoryBatch(String user, String project) {
	String temp = dirBatch;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetDirectoryScreen(String user, String project) {
	String temp = dirScreen;
	temp.replace("{u}", user);
	temp.replace("{p}", project);
	return temp;
}
String MemoryClass::GetAllocateFileName(String user, String project) {
	String temp = user + "/" + project + "/Allocate.homeos";
	return temp;
}
String MemoryClass::GetFileName(String name, String type) {
	if (type == "S") return "Str_" + name + ".hos";
	if (type == "F") return "Func_" + name + ".hos";
	return "";
}
void MemoryClass::SetCurrentUser(String user, String project) {
	CurrentUser = user;
	CurrentProject = project;
}

String MemoryClass::Encode(String strInput) {
	strInput.replace("00000000000000", "A9");
	strInput.replace("0000000000000", "A8");
	strInput.replace("000000000000", "A7");
	strInput.replace("00000000000", "A6");
	strInput.replace("0000000000", "A5");
	strInput.replace("00000000", "A4");
	strInput.replace("0000000", "A3");
	strInput.replace("000000", "A2");
	strInput.replace("00000", "A1");
	strInput.replace("0000", "A0");
	strInput.replace("111111111111", "B9");
	strInput.replace("11111111111", "B8");
	strInput.replace("1111111111", "B7");
	strInput.replace("111111111", "B6");
	strInput.replace("11111111", "B5");
	strInput.replace("1111111", "B4");
	strInput.replace("111111", "B3");
	strInput.replace("11111", "B2");
	strInput.replace("1111", "B1");
	strInput.replace("111", "B0");

	strInput.replace("11", "a"); strInput.replace("22", "b");
	strInput.replace("33", "c"); strInput.replace("44", "d");
	strInput.replace("55", "e"); strInput.replace("66", "f");
	strInput.replace("77", "g"); strInput.replace("88", "h");
	strInput.replace("99", "i"); strInput.replace("00", "j");

	strInput.replace("01", "k"); strInput.replace("02", "l");
	strInput.replace("03", "m"); strInput.replace("04", "n");
	strInput.replace("05", "o"); strInput.replace("06", "p");
	strInput.replace("07", "q"); strInput.replace("08", "r");
	strInput.replace("09", "t");
	strInput.replace("12", "s"); strInput.replace("13", "u");
	strInput.replace("14", "v"); strInput.replace("15", "x");
	strInput.replace("16", "y"); strInput.replace("17", "w");
	strInput.replace("18", "z"); strInput.replace("19", "_");

	strInput.replace("10", "="); strInput.replace("20", "!");
	strInput.replace("30", "@"); strInput.replace("40", "#");
	strInput.replace("50", "$"); strInput.replace("60", "%");
	strInput.replace("70", "^"); strInput.replace("80", "&");
	strInput.replace("90", "*");
	// ,.<>?+:;"
	strInput.replace("21", " "); strInput.replace("23", ",");
	strInput.replace("24", "."); strInput.replace("25", ">");
	strInput.replace("26", "<"); strInput.replace("27", "?");
	strInput.replace("28", "+"); strInput.replace("29", ":");
	//'`~()[]{}|/
	strInput.replace("31", "'"); strInput.replace("32", "`");
	strInput.replace("34", "~"); strInput.replace("35", "(");
	strInput.replace("36", ")"); strInput.replace("37", "[");
	strInput.replace("38", "]"); strInput.replace("39", "{");
	//\/|"0-;}
	strInput.replace("41", "Z"); strInput.replace("42", "/");
	strInput.replace("43", "|"); strInput.replace("45", "\"");
	strInput.replace("46", "C"); strInput.replace("47", "-");
	strInput.replace("48", ";"); strInput.replace("49", "}");
	//0DEFGHIJK
	strInput.replace("51", "D"); strInput.replace("52", "E");
	strInput.replace("53", "F"); strInput.replace("54", "G");
	strInput.replace("56", "H"); strInput.replace("57", "I");
	strInput.replace("58", "J"); strInput.replace("59", "K");
	//LMNOPQRS
	strInput.replace("61", "L"); strInput.replace("62", "M");
	strInput.replace("63", "N"); strInput.replace("64", "O");
	strInput.replace("65", "P"); strInput.replace("67", "Q");
	strInput.replace("68", "R"); strInput.replace("69", "S");
	//TUVWXYZ
	strInput.replace("71", "T"); strInput.replace("72", "U");
	strInput.replace("73", "V"); strInput.replace("74", "W");
	strInput.replace("75", "X"); strInput.replace("76", "Y");
	return strInput;
}
MemoryClass memo;