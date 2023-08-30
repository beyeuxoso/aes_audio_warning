
#include "WSHandle.h"
#include "HelpString.h"
#include "Config.h"
#include "Dxdiag.h"
#include "Dbg.h"
#include "SDCard.h"
#include "GPIO.h"
#include "RTC_DS3231.h"
#include "ntp.h"
#include "AppWiFi.h"
#include "Memory.h"
#include "Watchdog.h"
#include "Fota.h"
#include "HLW8012.h"
#include "serial_handle.h"

//-------------------------------------------------------------------------
//								Code a Hưng
//-------------------------------------------------------------------------
//...
//Bật/tắt chế độ chuyển tiếp dữ liệu qua TCP
bool Forwarding_Serial /*                                         */ = false;

//Bật chế độ xử lý dữ liệu nhận được từ Serial, SERIAL_PROCESS, giá trị: 1 -> xử lý và trả kết quả, 2-> xử lý và không trả kết quả, 0 -> không xử lý
uint8_t TurnOnProcessSerial/*                                     */ = 1;

//Bật chế độ ghi log vào thẻ nhớ
bool TurnOnWriteLogSerial/*                                       */ = true;

//Phải bật tính năng này thì ListSerialEventData mới có hiệu lực
bool EnableSerialEvent = false;
String ReadSerial = "";
String dataSerial = "";
//Thông báo đang ở trạng thái khởi động sau reset, ra lệnh thực hiện bắt buộc với những lệnh cần thực thi
bool isStartup/*                                                  */ = true;
//Thông báo đã nhận đc tín hiệu phản hồi từ server khi kết nối Websocket, nếu = false tức là chưa giao tiếp đc với server.
bool isServerAsk/*                                                */ = false;
//Nhận biết có kết nối internet hay không
//0 - không có kết nối
//1 - Đã kết nối
//2 - Đã từng kết nối
uint8_t IsConnectedInternet /*                                    */ = 0;

//Bật/tắt chế độ chuyển tiếp qua websocket
bool Websocket_Serial /*                                          */ = false;

//Bật chế độ xử lý dữ liệu nhận được từ Serial
bool TurnOnProcessWebsocket/*                                     */ = true;

//Chỉ mục WS hiện tại đang giao tiếp dữ liệu
uint8_t CurrentUWS /*                                             */ = 0;
//Biến cục bộ để lưu trữ websocket client nào đang được phép giao tiếp với thiết bị
uint8_t _a2s /*                                                   */ = 200;

//Nhận biến có dữ liệu từ Websocket
uint8_t HasMessageWS /*                                           */ = 0;

uint8_t TotalSocketClient = 0;

float VERSION = 5.30;
String Http_Result = "";
String MainCenter = "";
String DirLogger = "";

//Trạng thái thẻ SD
bool SD_INITIAL /*                                                */ = true;
//Xác định xem có lấy cả file khi đọc hay ko
uint8_t boolIncludeFile/*                                         */ = 1;

//Thông báo đang sử dụng while bất đồng bộ
bool isAwhile /*                                                  */ = false;
String CacheCallBatch = "";
//String CommandCallBatch = "";
String CacheCallBatchCmd = "";

#pragma region Process Batch
void WS_helpBatchProc(String data)
{
	String ret;
	data.trim();
	SDCard_helpParseBatchFile(data, &ret);
	WSClient_PushFiFoBuffer(ret);
}

String ProcessBatchData(String data)
{
	int pos = 0;
	String command = "";
	String batchFilenme = "";
	String result = "";
	while ((pos = data.indexOf(';')) != -1) 
	{
		// Trích xuất chuỗi con từ đầu đến vị trí của dấu ';'
		command = data.substring(0, pos + 1);
		command.trim();
		// Xóa chuỗi con đã được trích xuất và dấu ';'
		data.remove(0, pos + 1);
		String cmd = "CALL BATCH";
		if (command.startsWith(cmd))
		{
			command.remove(0, cmd.length());
			command.replace(";", "");
			command.trim();
			batchFilenme = RootDir + BatchDir + command + ".hbat";
			SDCard_helpParseBatchFile(batchFilenme, &result);
		}
		else
		{
			if (!command.startsWith("//"))
			{
				result += command;
			}
		}
	}
	WSClient_PushFiFoBuffer(result);
	return DONE;
}
#pragma endregion

#pragma region TimeExecute
typedef struct 
{
	String Name = "";
	String Command = "";
	uint32_t Interval = 0;
	uint32_t CountDown = 0;
	bool isTimeout = false;
} TimerExecute;
//Điều chỉnh Watchdog khi treo hệ thống
uint32_t timeoutWDT	= 6000U;
//Điều chỉnh tần số quét timer để thực hiện 1 trong 10 lệnh
uint32_t timeout_Timer = 1U;
LinkedList<TimerExecute> ListTimeExecute;

void ExecuteTimeProc_Main() {
	if (ListTimeExecute.size() <= 0) {
		return;
	}
	for (uint8_t i = 0; i < ListTimeExecute.size(); i++)
	{
		if (ListTimeExecute.get(i).Name != "") {
			if (ListTimeExecute.get(i).isTimeout) {
				if (ListTimeExecute.get(i).CountDown > 0)
				{
					TimerExecute te = ListTimeExecute.get(i);
					te.CountDown -= timeout_Timer;
				}
				else
				{
					ESP.wdtDisable();
					ProcessBatchData(ListTimeExecute.get(i).Command);
					ListTimeExecute.remove(i);
					if (i < ListTimeExecute.size() && i>0) i--;
					ESP.wdtEnable(timeoutWDT);
				}
			}
			else
			{
				if (ListTimeExecute.get(i).CountDown > 0) ListTimeExecute[i].CountDown -= timeout_Timer;
				else
				{
					ESP.wdtDisable();
					ProcessBatchData(ListTimeExecute.get(i).Command);
					TimerExecute te;
					te = ListTimeExecute.get(i);
					te.CountDown = ListTimeExecute.get(i).Interval;
					ListTimeExecute.set(i, te);
					ESP.wdtEnable(timeoutWDT);
				}
			}
		}
	}
}
#pragma endregion

#pragma region Memory OnEvent
PreprocessClass preprocessor;
bool createDirectory(String dir) {
#if USING_SDCARD
	return SD.mkdir(dir);
#else
	return false;
#endif
}
void IRAM_ATTR CreateUser(String user, String project) {
	createDirectory(memo.GetDirectoryString(user, project));
	createDirectory(memo.GetDirectoryFunction(user, project));
	createDirectory(memo.GetDirectoryBatch(user, project));
}
static void declare() {
	if (*memo.GetValue("COUNTER") == "") {
		memo.Declare(F("BYTE ACTIVE_2WAY_WS=0"));
		memo.Let(F("CHAR[19] ISO_DTIME='2021-01-01 00:00:00'"));
		memo.Declare(F("NUMBER[5] COUNTER=0"));
		memo.Declare(F("NUMBER[5] TIMEOUT=20"));
		memo.Declare(F("BOOL FORWARD_SERIAL=0"));
		memo.Declare(F("BOOL DEBUG=1"));
		memo.Declare(F("BOOL TURN_AP=1"));
		memo.Declare(F("CHAR[32] MAINCENTER=' '"));
		memo.Declare(F("CHAR[32] PASSWORD_AP='Blank'"));
		memo.Declare(F("CHAR[32] SSID_AP='HomeOS.vn'"));
		memo.Declare(F("CHAR[32] DEVICE_NAME='August 01'"));
		memo.Declare(F("NUMBER[5] PORT_WEBSERVER=80"));
		memo.Declare(F("NUMBER[5] PORT_WEBSOCKET=81"));
		memo.Declare(F("NUMBER[6] BAUDRATE=115200"));
		memo.SetValue("SSID_AP", "HomeOS.vn");
		memo.SetValue("PASSWORD_AP", "12345678");
	}
	else
		memo.SetValue("COUNTER", String((*memo.GetValue("COUNTER")).toInt() + 1));
	if (!memo.ExistName("HOMEOS"))
	{
		memo.Declare(F("SOCKETCLIENT websocket=ws://homeos.vn:81/forwarding"));
	}
	WiFi.hostname(*memo.GetValue("DEVICE_NAME"));
	Forwarding_Serial = (*memo.GetValue("FORWARD_SERIAL") == "1" ? true : false);
	MainCenter = *memo.GetValue("MAINCENTER");
	MainCenter.trim();
}

static void VariableSystem() {
	if (*memo.GetValue("CHIP_ID") == "" || *memo.GetValue("MAC_ADDRESS") == "") {
		memo.Let(F("NUMBER[2,4] VCC_DATA=0"));
		memo.Let(F("NUMBER[5] RSSI_DATA=0"));
		memo.Let(F("NUMBER[10] CHIP_ID=0"));
	}
}
void Memory_onListen()
{

}
void Memory_onEvent_Saved()
{

}

String Memory_onExpression(String data) {
	return preprocessor.Execute(data);
}

void Memory_onLoadMemory()
{

}

void Memory_onClearMemory()
{

}

void Memory_onSaveMemory(DynamicJsonDocument djd) {

}

void Memory_onSaveToSD(String name, String value, MemoryType memo_type) {
	String dir = memo.GetDirectoryString(memo.CurrentUser, memo.CurrentProject);
	String vname = memo.GetFileName(name, "S");

	if (SD.exists(dir + vname)) {
		SD.remove(dir + vname);
	}
	SDCard_WriteFile(dir + vname, value, "");
	if (memo_type == MemoryType::SOCKETCLIENT)
	{
		//AutoSocketClient();				//comment by toanpv1
	}
		
}

String Memory_onGetValue(AllocateAddress aa) {
	if (aa.Name.startsWith("_C_MILLIS_")) {
		return String(millis());
	}
	return aa.Value;
}

void Memory_onEvent_DataProcessed(String command, String data, String typeSending, String format) {

	if (format == "J") {
		DynamicJsonDocument doc(1024);
		doc["command"] = command;
		doc["data"] = data;
		doc["datetime"] = memo.DefaultValue(MemoryType::DATETIME);
		while (typeSending != "")
		{
			String cru = typeSending.subString(0, typeSending.indexOf(","));
			typeSending.remove(0, typeSending.indexOf(",") + 1);
			if (cru == "SERIAL") serializeJson(doc, Serial);
			if (cru == "ALL")
			{
				serializeJson(doc, cru);
				// ProcessData("SEND TCP ALL=" + cru, 9);			//comment by toanpv1

			}
			else {
				AllocateAddress aa = memo.GetPropertiesByName(cru);
				if (aa.Type == MemoryType::TCPCLIENT) {
					serializeJson(doc, cru);
					//ProcessData("SEND TCP ALL=" + cru, 9);		//comment by toanpv1
				}
			}
		}
	}
	else
	{
	}
}

String Memory_onGetValueFromSD(String name) {

	if (name == F("VERSION")) { return String(VERSION); }
	if (name == F("HTTP_RESULT")) { String data = Http_Result; Http_Result = ""; return data; }

	if (name == F("ENABLE_SERIAL_EVENT")) { EnableSerialEvent = true; return String(EnableSerialEvent); }
	if (name == F("DISABLE_SERIAL_EVENT")) { EnableSerialEvent = false; return String(EnableSerialEvent); }
	if (name == F("SERIAL_PROCESS")) { return String(TurnOnProcessSerial); }
	if (name == F("WEBSOCKET_PROCESS")) { return String(TurnOnProcessWebsocket); }
	if (name == F("WRITELOG_PROCESS")) { return String(TurnOnWriteLogSerial); }

	if (name == F("NETWORK_CONNECTED")) { return String(IsConnectedInternet); }
	if (name == F("DATA_SERIAL")) { return ReadSerial; }
	if (name == F("ISO_DTIME")) { datetime.trim(); return datetime; }
	if (name == F("ISO_TIME")) { strtime.trim(); return strtime; }
	if (name == F("ISO_STIME")) { strtime.trim(); strtime = strtime.subString(0, 5); return strtime; }
	if (name == F("ISO_DATE")) { strdate.trim(); return strdate; }
	if (name == F("IP_ADDRESS")) { return  WiFi.localIP().toString(); }
	if (name == F("GATEWAY")) { return  WiFi.gatewayIP().toString(); }
	if (name == F("SUBNET_MASK")) { return  WiFi.subnetMask().toString(); }
	if (name == F("MAC_ADDRESS")) { return  WiFi.macAddress().c_str(); }
	if (name == F("IRAM")) { return  String(ESP.getFreeHeap()); }
	if (name == F("CHIP_ID")) { return  String(ESP.getChipId()); }
	if (name == F("RSSI_DATA")) { return  String(WiFi.RSSI()); }
	if (name == F("BSSID_DATA")) { return  String(WiFi.BSSIDstr()); }
	if (name == F("VCC_DATA")) { return  String(ESP.getVcc()); }

	String dir = memo.GetDirectoryString(memo.CurrentUser, memo.CurrentProject);

	String content = SDCard_LoadFile(dir + memo.GetFileName(name, "S"));
	if (content == "")
	{
		dir = memo.GetDirectoryFunction(memo.CurrentUser, memo.CurrentProject);
		content = SDCard_LoadFile(dir + memo.GetFileName(name, "D"));
	}
	return content;
}

void Memory_Initial()
{
	CreateUser(memo.CurrentUser, memo.CurrentProject);
	memo.LoadMemory();
	memo.onExpression(&Memory_onExpression);
	memo.onSaveToSD(&Memory_onSaveToSD);
	memo.onGetValueFromSD(&Memory_onGetValueFromSD);
	memo.onGetValue(&Memory_onGetValue);
	memo.onEvent_DataProcessed(&Memory_onEvent_DataProcessed);
	
	DirLogger = memo.GetDirectoryLogger(memo.CurrentUser, memo.CurrentProject);
	DirLogger.remove(DirLogger.lastIndexOf("/"), 1);
	SD.mkdir(DirLogger);
	DirLogger += "/";
	declare();
	VariableSystem();
}
#pragma endregion

#pragma region Preprocess
void RefreshMemory_OnRun() {
	preprocessor.memoryEx = memo;
}

void Preprocess_Init()
{
	Preprocess.init(memo);
	preprocessor.onRun(RefreshMemory_OnRun);
}

#pragma endregion
#pragma region Scheduling
typedef struct {
	int Timer = 0;
	String Name = "";
	String Start_Time = "";
	String End_Time = "";
	bool isloop = false;
	String Event = "";
} Scheduling;

LinkedList<Scheduling> ListSchedule = LinkedList<Scheduling>();

bool ExistSchedule(String name) {
	for (uint i = 0; i < ListSchedule.size(); i++)
	{
		if (ListSchedule.get(i).Name == name)return true;
	}
	return false;
}
void CreateSchedule(String name, String starttime, String endtime, bool isloop, String e) {
	if (!ExistSchedule(name)) {
		Scheduling row;
		row.Name = name;
		row.Start_Time = starttime;
		row.End_Time = endtime;
		row.isloop = isloop;
		row.Timer = 0;
		row.Event = e + ";";
		ListSchedule.add(row);

	}
	else
	{
		for (uint i = 0; i < ListSchedule.size(); i++)
		{

			if (ListSchedule.get(i).Name == name) {
				Scheduling s = ListSchedule.get(i);
				s.Name = name;
				s.Start_Time = starttime;
				s.End_Time = endtime;
				s.isloop = isloop;
				s.Timer = 0;
				s.Event = e + ";";
				ListSchedule.set(i, s);
				break;
			}
		}
	}
}
void DeleteSchedule(String name) {
	for (uint i = 0; i < ListSchedule.size(); i++)
	{
		if (ListSchedule.get(i).Name == name) {
			ListSchedule.remove(i);
			return;
		}
	}
}

void ExecuteSchedule_Main() {
	if (ListSchedule.size() <= 0) return;
	for (size_t i = 0; i < ListSchedule.size(); i++)
	{
		ulong st = longTime(ListSchedule.get(i).Start_Time);
		ulong et = longTime(ListSchedule.get(i).End_Time);
		//23:54:00 > 01:20:12  (23:59:00) => trong khoảng nhưng vắt qua ngày
		ulong cr = longTime(strtime);
		if (st > et) {
			if (st <= cr && cr <= 235959) {
				if (ListSchedule.get(i).isloop == true)
				{
					Scheduling s = ListSchedule.get(i);
					s.Timer++;
					ListSchedule.set(i, s);
					ProcessBatchData(ListSchedule.get(i).Event);
				}
				else
				{
					if (ListSchedule.get(i).Timer == 0) {
						ProcessBatchData(ListSchedule.get(i).Event);
					}
				}
			}
			if (et > cr && cr > 235959 && ListSchedule.get(i).isloop == true)
			{
				ProcessBatchData(ListSchedule.get(i).Event);
			}
		}
		else
		{
			if (st <= cr && cr <= et) {
				if (ListSchedule.get(i).isloop == true)
				{
					Scheduling s = ListSchedule.get(i);
					s.Timer++;
					ListSchedule.set(i, s);
					ProcessBatchData(ListSchedule.get(i).Event);
				}
				else
				{
					if (ListSchedule.get(i).Timer == 0) {
						ProcessBatchData(ListSchedule.get(i).Event);
					}
				}
			}
		}
	}
}
#pragma endregion
//-------------------------------------------------------------------------
//								Process
//-------------------------------------------------------------------------
#pragma region Process Null
String ICACHE_FLASH_ATTR nullProc(String data)
{
	return "Unknown";
}

String ICACHE_FLASH_ATTR CommentProc(String data)
{
	return "Comment";
}
String ICACHE_FLASH_ATTR ReturnProc(String data)
{
	return AES_WSKEY;
}
String ICACHE_FLASH_ATTR NoneProc(String data)
{
	return NONE;
}
#pragma endregion

#pragma region Process Set
// Set command
/*
* VN: Đặt giá trị cho 1 biến đã khai báo, có thể tìm biến đã khai báo thông qua lênh LR.
*/
String TypeSetVN(String name, String value, String event) {
	if (value != "NULL")
	{
		if (name == "FORWARD_SERIAL")	Forwarding_Serial = (value == "1" ? true : false);
		if (name == "WEBSOCKET_SERIAL")	Websocket_Serial = (value == "1" ? true : false);
		if (name == "SERIAL_PROCESS")	TurnOnProcessSerial = value.toInt();
		if (name == "WEBSOCKET_PROCESS")TurnOnProcessWebsocket = (value == "1" ? true : false);
		if (name == "WRITELOG_PROCESS")	TurnOnWriteLogSerial = (value == "1" ? true : false);
#if USING_ROTARY
		if (name == "RPHASE_A")encoder0PinA = value.toInt();
		if (name == "RPHASE_B")encoder0PinB = value.toInt();
#endif
		memo.Set(name + "=" + value);
	}
	if (event != "" && event != "NULL") {
		event += ";";
		return ProcessBatchData(event);
	}
	return DONE;
}

/*
* TI: Lệnh cài đặt lại thời gian thực, trong đó tên biến là NULL, 
*	  value là giá trị thời gian cần đặt theo định dạng yyyy-mm-dd hh:MM:ss.
*/
String TypeSetTI(String name, String value, String event) {
	tm time;
	time.tm_sec = value.subString(17, 2).toInt();
	time.tm_min = value.subString(14, 2).toInt();
	time.tm_hour = value.subString(11, 2).toInt();
	time.tm_mday = value.subString(8, 2).toInt();
	time.tm_mon = value.subString(5, 2).toInt();
	time.tm_year = value.subString(0, 4).toInt() - 2000;
	
	RTC_DS3231_Set(&time);

	if (name != "" && name != "NULL") memo.Set(name + "='" + value + "'");
	if (event != "" && event != "NULL")
	{
		event += ";";
		return ProcessBatchData(event);
	}
	return NONE;
}
/*
* CI: CI xoá 1 lệnh interval hoăc timeout đã đặt trước đó, với name là tên biến 
*	  của lệnh timeout hoặc interval đã đặt ở trên, các giá trị còn lại đặt NULL.
*/
String TypeSetCI(String name) {
	for (uint8_t i = 0; i < ListTimeExecute.size(); i++)
	{
		if (ListTimeExecute[i].Name == name) {
			ListTimeExecute.remove(i);
			break;
		}
	}
	return DONE;
}

/*
* IV: Khai báo 1 interval,cho phép lặp lại 1 lệnh trong 1 khoảng thời gian chỉ định, 
*     khi đó, tên biến là 1 tên tự đặt(không liên quan tới tên biến trong LR), 
*	  value là chu kì lắp tính theo ms. event là lệnh sẽ thực hiện sau mỗi chu kì lặp.
* TO: Khai báo 1 Timeout,cho phép lặp lại 1 lệnh sau 1 thời gian trễ, lệnh chỉ thực hiện 1 lần, 
*	  khi đó, tên biến là 1 tên tự đặt(không liên quan tới tên biến trong LR), 
*	  value là chu kì trễ theo ms. event là lệnh sẽ thực hiện sau thời gian trễ.
* Syntax:
*		SET(IV,test,1000,SEND ESERIAL('Hello world!!!')); //Cú pháp dành cho tần suất gửi;
*		SET(TO,test,1000,SEND ESERIAL('Hello world!!!')); //Cú pháp dành cho gửi 1 lần;
*		func: TypeSetIV_TO
*		params: name-> tên của lệnh, value-> giá trị đếm ngược, event-> lệnh thực thi, isTO-> chỉ gửi 1 lần, typeSend-> loại gửi từ phương thức nào
*/
String TypeSetIV_TO(String name, String value, String event, bool isTO) {
	bool hasData = false;
	for (uint8_t i = 0; i < ListTimeExecute.size(); i++)
	{
		if (ListTimeExecute.get(i).Name == "" || ListTimeExecute.get(i).Name == name) {
			TimerExecute te = ListTimeExecute.get(i);
			te.Name = name;
			te.Command = event + ";";
			te.isTimeout = isTO;
			te.Interval = (isTO ? 0 : value.toDouble());
			te.CountDown = value.toDouble();
			ListTimeExecute.set(i, te);
			hasData = true;
			break;
		}
	}
	if (!hasData) {
		TimerExecute te;
		te.Name = name;
		te.Command = event + ";";
		te.isTimeout = isTO;
		te.Interval = (isTO ? 0 : value.toDouble());
		te.CountDown = value.toDouble();
		ListTimeExecute.add(te);
	}
	return DONE;
}

String SETProc(String data)
{
	data = trimIndex(data, " ");
	String name = getIndex(data, "=");
	name.trim();
	data = trimIndex(data, "=");
	String value = data;
	value.trim();
	if (value != "NULL")
	{
		return TypeSetVN(name, value, "");
	}
	return DONE;
}
String SETProc_1(String data)
{
	data = trimIndex(data, "(");
	String t = getIndex(data, ","); data = trimIndex(data, ",");
	String name = getIndex(data, ","); data = trimIndex(data, ",");
	String value = getIndex(data, ","); data = trimIndex(data, ",");
	String eventt = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
	bool hasData = false;
	//Theo biến khai báo
	if (t == "VN") {
		return TypeSetVN(name, value, eventt);
	}
	//Theo interval
	if (t == "EA") {
		if (eventt != "NULL") {
			//eventADC = eventt;
		}
		return DONE;
	}
	//Theo timeout, chỉ thực hiện 1 lần dựa trên timeout
	if (t == "TO" || t == "IV") {
		return TypeSetIV_TO(name, value, eventt, (t == "TO" ? true : false));
	}
	//Theo timeout, chỉ thực hiện 1 lần dựa trên timeout 
	if (t == "CI") {
		return TypeSetCI(name);
	}
	//Theo time
	if (t == "TI") {
		return TypeSetTI(name, value, eventt);
	}
	return DONE;
}
#pragma endregion

#pragma region Process Delay
String DelayMSProc(String data)
{
	String command = "DELAY_MS ";
	data.remove(0, command.length());
	data.trim();
	if (!memo.isNumberic(data)) data = *memo.GetValue(data);
	ESP.wdtDisable();
	Dbg_Printf("Delay ms cmd: ");
	Dbg_Println(data.toInt());
	delay(data.toInt());
	ESP.wdtEnable(timeoutWDT);
	return DONE;
}

String DelayUSProc(String data)
{
	String command = "DELAY_US ";
	data.remove(0, command.length());
	data.trim();
	ESP.wdtDisable();
	if (!memo.isNumberic(data)) data = *memo.GetValue(data);
	ets_delay_us(data.toDouble());
	ESP.wdtEnable(timeoutWDT);
	return DONE;
}
#pragma endregion

#pragma region Process Serial
String ClearSerialProc(String data)
{
	return NOT_SUPPORT;
}

String WaitSerialProc(String data)
{
	return NOT_SUPPORT;
}

String AwaitSerialProc(String data)
{
	return NOT_SUPPORT;
}
String RemoveSerialProc(String data)
{
	return NOT_SUPPORT;
}
#pragma endregion

#pragma region Process LoadUI
String LoadScreenProc(String data)
{
	String command;
	command = "LOAD SCREEN ";
	// Get file name
	data.remove(0, command.length()); 
	data.trim();
	data.replace(";", "");
	// Get dir
	String dir = RootDir + ScreenDir + data + ".hscrn";
	dir.trim();
	// Load data from SD Card
	data = SDCard_LoadFile(dir);
	if (data != "") 
	{
		// Send to Websocket server
		String a = "Command: #BEGIN FILE, ";
		WSClient_sendMsg(a.c_str());
		WSClient_sendMsg(data.c_str());
		a = "Command: #END FILE";
		WSClient_sendMsg(a.c_str());
		return DONE;
	}	
	else 
	{
		return "NO SCREEN TO LOAD";
	}
}

String LoadUIProc(String data)
{
	String command;
	command = "LOAD UI ";
	// Get file name
	data.remove(0, command.length());
	data.trim();
	data.replace(";", "");
	// Get dir
	String dir = RootDir + ScreenDir + data + ".hscrn";
	dir.trim();
	// Load data from SD Card
	data = SDCard_LoadFile(dir);
	if (data != "")
	{
		// Send to Websocket server
		String a = "Command: #BEGIN FILE, ";
		WSClient_sendMsg(a.c_str());
		WSClient_sendMsg(data.c_str());
		a = "Command: #END FILE";
		WSClient_sendMsg(a.c_str());
		return DONE;
	}
	else
	{
		return "NO SCREEN TO LOAD";
	}
}
#pragma endregion

#pragma region DxDiag
String DxdiagGetFileSizeProc(String data)
{
	String command = "GET SIZE FILE ";
	
	if (data.startsWith(command))
	{
		data.remove(0, command.length()); 
		data.trim();
		File file = SD.open(data, FILE_READ);
		if (file.available()) {
			String fname = String(file.name());
			String fsize = String(file.size());
			file.close();
			return fname + ", size: " + fsize;
		}
		else
		{
			return "File unvailable";
		}
	}
	return NONE;
}
#pragma endregion

#pragma region Pin Process
String PinReadDIProc(String data)
{
	String ret;
	String command = "DIGITAL READ ";
	data.remove(0, command.length());
	data.trim();
	String name = getIndex(data, " ");
	data.remove(0, name.length() + 1);
	data.trim();
	String pinName = name;
	uint16_t value = 0xFF;
	// Digital Write Process
	ret = GPIO_Process(pinName, &value, 0xFF, DIGITAL_READ);
	if (strcmp(ret.c_str(), DONE) == 0)
	{
		ret = String(value);
	}
	return ret;
}
String PinWriteDIProc(String data)
{
	String ret;
	String command = "DIGITAL WRITE ";
	data.remove(0, command.length()); 
	data.trim();
	String name = getIndex(data, " ");
	data.remove(0, name.length() + 1);
	data.trim();
	String pinName = name;
	int _value = (preprocessor.Execute(data)).toInt();
	uint16_t value = _value < 0 ? 0 : _value;
	value = _value > 1 ? 1 : _value;
	// Digital Write Process
	ret = GPIO_Process(pinName, &value, OUTPUT, DIGITAL_WRITE);
	return ret;
}

String PinWriteANProc(String data)
{
	String ret;
	String command = "ANALOG WRITE ";

	data.remove(0, command.length()); 
	data.trim();
	String name = data.subString(0, data.indexOf(" "));
	data.remove(0, name.length() + 1); 
	data.trim();

	String pinName = name;
	int _value = (preprocessor.Execute(data)).toInt();
	uint16_t value = _value < 0 ? 0 : _value;
	// Analog Write Process
	ret = GPIO_Process(pinName, &value, NOT_USED, ANALOG_WRITE);
	return ret;
}

String PinReadANProc(String data)
{
	String ret = FAILED;
	String command = "ANALOG READ ";

	data.remove(0, command.length());
	data.trim();
	String name = data.subString(0, data.indexOf(" "));
	data.remove(0, name.length() + 1);
	data.trim();

	String pinName = name;
	uint16_t value;
	// Digital Read Process
	ret = GPIO_Process(pinName, &value, NOT_USED, ANALOG_READ);
	if (strcmp(ret.c_str(), DONE) == 0)
	{
		ret = String(value);
	}
	return ret;
}
#pragma endregion

#pragma region Load Ram
typedef struct {
	uint8_t index = 0;
	String data;
	///0 - so sánh bằng
	///1 - startwith
	///2 - endwidth
	///3 - contain
	uint8_t type = 0;
	bool loop = false;
	String command;
} SerialDataEvent;
LinkedList<SerialDataEvent> ListSerialEventData = LinkedList<SerialDataEvent>();
#define LIST_FILE_BUFER_SIZE				(1024U)

static String helpLRListFileProc(String dir, String exFile)
{
	char List[LIST_FILE_BUFER_SIZE];
	uint16_t num = SDCard_ListFile(dir, List, exFile);
	if (num > 0)
	{
		String fName = "";
		for (uint16_t i = 0; i < LIST_FILE_BUFER_SIZE; i++)
		{
			if (List[i] == ';')
			{
				WSClient_sendMsgLen(fName.c_str(), fName.length());
				fName = "";
			}
			else
			{
				fName += List[i];
			}
			if (List[i] == '\0') break;
		}
		return DONE;
	}
	return NONE;
}

static String LR_Schedule(String data) {
	String f = getIndex(data, ","); data = trimIndex(data, ",");
	String t = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
	String res = "";
	if (f.toInt() > ListSchedule.size()) return "Not Found";
	if (t.toInt() > ListSchedule.size()) t = String(ListSchedule.size());
	res += "Total: " + t + "/" + String(ListSchedule.size()) + "\r\n";
	if (t == "NULL") t = "999";
	if (t.toInt() > ListSchedule.size())t = String(ListSchedule.size());
	for (uint8_t i = f.toInt(); i < ListSchedule.size(); i++)
	{
		res += "\r\n[" + String(i).padleft(2, "0") + "]:" +
			ListSchedule[i].Name.padleftString(16, " ") +
			ListSchedule[i].Start_Time.padleftString(10, " ") +
			ListSchedule[i].End_Time.padleftString(10, " ") +
			String(ListSchedule[i].isloop).padleft(3, " ") +
			String(ListSchedule[i].Timer).padleft(3, " ") +
			ListSchedule[i].Event.padleftString(64, " ");
	}
	if (res == "") res = "Not Found";
	return res;
}

//static String LR_Pinmode(String data) {
//	String f = getIndex(data, ","); data = trimIndex(data, ",");
//	String t = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
//	String res = "";
//	PinRuntime pr;
//	if (t == "NULL") t = "999";
//	if (t.toInt() > ListPinRuntime.size())t = String(ListPinRuntime.size());
//	for (uint8_t i = f.toInt(); i < t.toInt(); i++)
//	{
//		pr = ListPinRuntime[i];
//		res += "\r\n[" + String(i).padleft(2, "0") + "]:" +
//			pr.Name.ColumnWidth(16, LEFT) +
//			String(pr.Pin).ColumnWidth(5, RIGHT) + "   " +
//			pr.Func.ColumnWidth(64, LEFT) +
//			String(pr.hasInterupt).ColumnWidth(5, RIGHT) + "   " +
//			String(pr.isOut).ColumnWidth(5, RIGHT) + "   " +
//			String(pr.Millis).ColumnWidth(10, RIGHT) + "   " +
//			String(pr.typeInterupt).ColumnWidth(5, RIGHT) + "   " +
//			String(pr.Type).ColumnWidth(5, RIGHT);
//	}
//	if (res == "") res = "Not Found";
//	return res;
//}

static String LR_Interval(String data) {
	String f = getIndex(data, ","); data = trimIndex(data, ",");
	String t = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
	String res = "";
	TimerExecute lte;
	if (t == "NULL") t = "999";
	if (t.toInt() > ListTimeExecute.size())t = String(ListTimeExecute.size());
	for (uint8_t i = f.toInt(); i < t.toInt(); i++)
	{
		lte = ListTimeExecute.get(i);
		res += "\r\n[" + String(i).padleft(2, "0") + "]:" +
			lte.Name.padleftString(16, " ") +
			String(lte.Command).padleftString(64, " ") +
			String(lte.isTimeout).padleft(5, " ") + "   " +
			String(lte.Interval).padleft(10, " ") + "   " +
			String(lte.CountDown).padleft(10, " ");
	}
	if (res == "") res = "Not Found";
	return res;
}

static String LR_Serial(String data) {
	String f = getIndex(data, ","); data = trimIndex(data, ",");
	String t = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
	String res = "";
	SerialDataEvent pr;
	if (t == "NULL") t = "999";
	if (t.toInt() > ListSerialEventData.size())t = String(ListSerialEventData.size());
	for (uint8_t i = f.toInt(); i < t.toInt(); i++)
	{
		pr = ListSerialEventData.get(i);
		res += "\r\n[" + String(i).padleft(2, "0") + "]:" +
			String(pr.index).ColumnWidth(5, RIGHT) + "   " +
			pr.command.ColumnWidth(64, LEFT) +
			pr.data.ColumnWidth(32, LEFT) +
			String(pr.loop).ColumnWidth(5, RIGHT) + "   " +
			String(pr.type).ColumnWidth(5, RIGHT);
	}
	if (res == "") res = "Not Found";
	return res;
}

String LrBatchProc(String data) 
{
	String command = "LR BATCH";
	if (data.startsWith(command))
	{
		String dir = RootDir + BatchDir;
		return helpLRListFileProc(dir, ".hbat");
	}

	command = "LR SERIAL(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		return LR_Serial(data);
	}
	command = "LR SCHEDULE(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		return LR_Schedule(data);
	}
	//command = "LR PINMODE(";
	//if (data.startsWith(command))
	//{
	//	data.remove(0, command.length());
	//	data.trim();
	//	return LR_Pinmode(data);
	//}
	command = "LR WEBSOCKET";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();

		return "TOTAL WEBSOCKET:" + String(TotalSocketClient);
	}
	command = "LR INTERVAL(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		return LR_Interval(data);
	}
	command = "LR 74HC595(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String f = getIndex(data, ","); data = trimIndex(data, ",");
		String t = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
		String res = "";
		if (res == "") res = "Not Found";
		return res;
	}
	command = "LR INDEX ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		return memo.GetVariable(data.toInt());
	}
	command = "LR FROM ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String from = "0", to = "0";
		from = data.subString(0, data.indexOf("TO"));
		from.trim();
		data.remove(0, data.indexOf("TO") + 2);
		to = data;
		to.trim();
		String kq = "";
		for (uint i = from.toInt(); i <= to.toInt(); i++)
		{
			kq += memo.GetVariable(i);
		}
		if (kq == "") return "Not found";
		return kq;
	}
	command = "LR NAME ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		for (uint i = 0; i < MAX_VARIABLE; i++)
		{
			if (data == memo.ListAllocateAddress[i].Name) {
				return memo.GetVariable(i);
			}
		}
		return "Not found";
	}
	command = "LR TYPE ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String v = "";
		for (uint i = 0; i < MAX_VARIABLE; i++)
		{
			if (data.toInt() == memo.GetAddressByType(memo.ListAllocateAddress[i].Type)) {
				v += memo.GetVariable(i);
			}
		}
		if (v == "")
			return "Not found";
		else
			return v;
	}
	return NONE;
}

String LrScreenProc(String data)
{
	String dir = RootDir + ScreenDir;
	return helpLRListFileProc(dir, ".hscrn");
}

String LrStartupProc(String data)
{
	String dir = RootDir;
	return helpLRListFileProc(dir, ".hos");
}
#pragma endregion

#pragma region WatchDog
typedef enum
{
	WDT_DISABLE,
	WDT_ENABLE,
	WDT_FEED,
	WDT_INVALID,
} WdtType_t;

String WatchDogProc(String data)
{
	//WDT(1,4000,NULL);			// 4000ms
	String command = "WDT(";
	WdtType_t WdtType;
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		String state = getIndex(data, ","); data = trimIndex(data, ",");
		String sec = getIndex(data, ","); data = trimIndex(data, ",");
		String cmd = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
		if (state.toInt() < (uint8_t) WDT_INVALID)
		{
			WdtType = (WdtType_t)state.toInt();
		}
		switch (WdtType)
		{
			case WDT_ENABLE:
			{
				if (sec != "NULL") {
					Wdg_Enable((uint32_t)sec.toDouble());
				}
				else {
					Wdg_Enable(timeoutWDT);
				}
				break;
			}
			case WDT_DISABLE:
			{
				Wdg_Disable();
				break;
			}
			case WDT_FEED:
			{
				Wdg_Feed();
				break;
			}
		}
		if (cmd != "" && cmd != "NULL")
		{
			cmd += ";";
			ProcessBatchData(cmd);
		}
		return DONE;
	}
	return NONE;
}
String SoftWatchDogProc(String data)
{
	//SWDT(1,150,NULL);			// 150s
	String command = "SWDT(";
	WdtType_t WdtType;
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		String state = getIndex(data, ","); data = trimIndex(data, ",");
		String sec = getIndex(data, ","); data = trimIndex(data, ",");
		String cmd = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
		if (state.toInt() < (uint8_t)WDT_INVALID)
		{
			WdtType = (WdtType_t)state.toInt();
		}
		switch (WdtType)
		{
		case WDT_ENABLE:
		{
			if (sec != "NULL") {
				SoftWdg_Enable((uint32_t)sec.toDouble());
			}
			else {
				SoftWdg_Enable(TIMMER_CNT_150S);
			}
			break;
		}
		case WDT_DISABLE:
		{
			SoftWdg_Disable();
			break;
		}
		case WDT_FEED:
		{
			SoftWdg_Feed();
			break;
		}
		}
		if (cmd != "" && cmd != "NULL")
		{
			cmd += ";";
			ProcessBatchData(cmd);
		}
		return DONE;
	}
	return NONE;
}
#pragma endregion

#pragma region WiFi
bool hasSSID = false;
String RefreshSSID(bool isClean) {
	if (isClean)
	{
		WiFi.disconnect();
		wifiMulti.cleanAPlist();
	}
	for (uint8_t i = 0; i < MAX_VARIABLE; i++)
	{
		if (memo.ListAllocateAddress[i].Type == MemoryType::WIFI)
		{

			String info = *memo.GetValue(memo.ListAllocateAddress[i].Name);
			if (!wifiMulti.existsAP(info.subString(0, info.indexOf("/")).c_str(), info.substring(info.indexOf("/") + 1).c_str()))
				wifiMulti.addAP(info.subString(0, info.indexOf("/")).c_str(), info.substring(info.indexOf("/") + 1).c_str());   // add Wi-Fi networks you want to connect to
			hasSSID = true;
		}
	}
	WiFi.reconnect();
	return DONE;
}
String ProcessWifi(String data)
{
	String command = "REFRESH SSID ";
	if (data.startsWith(command))
	{
		data.remove(0, 13);
		data.trim();
		WiFi.disconnect();
		if (data == "1")
			RefreshSSID(true);
		else
			RefreshSSID(false);
		WiFi.reconnect();
		return DONE;
	}
	command = "DISCONNECT WIFI(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		String sec = getIndex(data, ","); data = trimIndex(data, ",");
		String cmd = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
		WiFi.disconnect();
		if (sec != "NULL")
		{
			ESP.wdtDisable();
			delay(sec.toDouble());
			ESP.wdtEnable(timeoutWDT);
			WiFi.reconnect();
		}
		if (cmd != "" && cmd != "NULL") ProcessBatchData(cmd);
		return DONE;
	}
	command = "RE-CONNECT WIFI";
	if (data.startsWith(command))
	{
		WiFi.reconnect();
		return DONE;
	}
	command = "CLEAR SSID";
	if (data.startsWith(command))
	{
		WiFi.disconnect();
		wifiMulti.cleanAPlist();
		AppWiFi_GetSSID();
		WiFi.reconnect();
		return DONE;
	}
	return "None";
}
#pragma endregion

#pragma region Scan WiFi, I2C
static String I2C_Scan(uint8_t addr) 
{
	uint8_t error, address;
	String result;
	Wire.setTimeout(100);
	uint8_t nDevices = 0;
	if (addr > 127)
		for (address = 1; address < 127; address++) 
		{
			Wire.beginTransmission(address);
			error = Wire.endTransmission();
			if (error == 0) {
				result += "Address:" + String(address) + "\r\n";
				nDevices++;
			}
			else if (error == 4) {
			}
		}
	else
	{
		Wire.beginTransmission(addr);
		error = Wire.endTransmission();
		if (error == 0) {
			result += "Address:" + String(addr) + "\r\n";
			nDevices++;
		}
		else if (error == 4) {
		}
	}
	if (nDevices == 0)
		result = "Not found";
	else
		result += "Total:" + String(nDevices);
	return result;
}
String TypeScan(String data)
{
	String command = "SCAN";
	data.remove(0, command.length());
	data.trim();

	command = "NETWORK";
	if (data.startsWith(command))
	{
		AppWiFi_ScanNetwork(NORMAL_DATA, &data);
		WSClient_sendMsg(data.c_str());
		return DONE;
	}

	command = "JNETWORK";
	if (data.startsWith(command))
	{
		WSClient_sendMsg("[");
		AppWiFi_ScanNetwork(JSON_DATA, &data);
		WSClient_sendMsg(data.c_str());
		WSClient_sendMsg("]");
		return DONE;
	}
	command = "RTC";
	if (data.startsWith(command))
	{
		return I2C_Scan(DS3231_I2C_ADDRESS);
	}
	command = "I2C";
	if (data.startsWith(command))
	{
		return I2C_Scan(128);
	}
	return NONE;
}
#pragma endregion

#pragma region FirmWare Process
String FW_DownloadProc(String data)
{
	String ret = NONE;
	String command = "FIRMWARE DOWNLOAD(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String type = getIndex(data, ",");	data = trimIndex(data, ",");
		String url = getIndex(data, ",");	data = trimIndex(data, ",");
		String user = getIndex(data, ",");	data = trimIndex(data, ",");
		String password = getIndex(data, ",");	data = trimIndex(data, ",");
		String fileName = getIndex(data, ",");	data = trimIndex(data, ",");
		String bufferSize = getIndex(data, ")");
		Fota_Download(type,url, user, password, fileName, bufferSize.toInt(), &ret);
	}
	return ret;
}
String FW_UpdateProc(String data)
{
	String command = "FIRMWARE UPDATE";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String dir = data;
		Dbg_Println(dir);
		Fota_Update(dir);
	}
	return DONE;
}
#pragma endregion

#pragma region Process Execute

String ProcessExecute(String data) {
	String command;
	command = "GET RESET INFO";
	if (data.startsWith(command))
	{
		return ESP.getResetInfo();
	}
	
	command = "DEEP SLEEP ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		Sleep(data);
		return DONE;
	}

	command = "GET TIME";
	if (data.startsWith(command))
	{
		return datetime;
	}

	command = "RSSI";
	if (data.startsWith(command))
	{
		return "RSSI: " + String(WiFi.RSSI()) + " dBm";
	}
	command = "BSSID";
	if (data.startsWith(command))
	{
		return "BSSID:" + WiFi.BSSIDstr();
	}
	command = "CM";
	if (data.startsWith(command))
	{
		memo.ClearAllMemory();
		memo.ResetAllocateAddress();
		return DONE;
	}
	command = "RESET PIN(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String name = getIndex(data, ","); trimIndex(data, ",");
		String state = getIndex(data, ","); trimIndex(data, ",");
		String timeout = getLastIndex(data, ","); trimLastIndex(data, ",");
		if (memo.isNumberic(data)) {
			if (state == "1") {
				ESP.wdtDisable();
				digitalWrite(data.toInt(), HIGH);
				delay(timeout.toDouble());
				digitalWrite(data.toInt(), LOW);
				ESP.wdtEnable(timeoutWDT);
			}
			else
			{
				ESP.wdtDisable();
				digitalWrite(data.toInt(), LOW);
				delay(timeout.toDouble());
				digitalWrite(data.toInt(), HIGH);
				ESP.wdtEnable(timeoutWDT);
			}
		}
		return DONE;
	}
	command = "RESET WS";
	if (data.startsWith(command))
	{
		//AutoSocketClient();		//comment by toanpv1
		return DONE;
	}
	command = "RESET";
	if (data.startsWith(command))
	{
		Dbg_Printf("System restart now!!!");
		ESP.reset();
		return DONE;
	}
	if (data.startsWith("SM")) {
		memo.isSM = true;
		return DONE;
	}
	if (data.startsWith("SAVE")) {
		memo.SaveMemory();
		return  DONE;
	}
	
	command = "GET UIP";
	if (data.startsWith(command))
	{
#if USING_HLW8012
		data = HLW8012_Get();
		WSClient_sendMsg(data.c_str());
#endif
		return DONE;
	}
	command = "GET STATE LED";
	if (data.startsWith(command))
	{
		String data = "";
		String _state = *memo.GetValue("STATE_LED");
		String _dim = *memo.GetValue("DIMM_LED");
		data = "T:" + datetime + ", S:" + _state + ", D:" + _dim;
		WSClient_sendMsg(data.c_str());
		return DONE;
	}

	command = "GET ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String value = "";
		if (data.startsWith("http://") || data.startsWith("https://")) {
			String header = "";
			if (memo.ExistName("DEFINE_HEADER"));
			header = *memo.GetValue("DEFINE_HEADER");
			header.trim();
			//value = CallAPI("GET", data, 5000, header, false);		//comment by toanpv1
			/*	value.remove(0, 1);
				value.remove(value.length() - 1, 1);*/
			return value;
		}
		value = *memo.GetValue(data);
		return value;
	}
	command = "DISPOSE ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		memo.Dispose(data);
		return DONE;
	}
	command = "JOIN ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		String c = *memo.GetValue(data.split(",", 0)) + "" + *memo.GetValue(data.split(",", 1));
		return c;
	}
	return NONE;
}

String UpdateTimeProc(String data)
{
	tm time;
	NTP_Gettime(&time);
	RTC_DS3231_Set(&time);
	return DONE;
}
String ResetProc(String data)
{
	// Safety process
	Dbg_Printf("Reset by websocket command");
	// Reset
	ESP.reset();
	return NONE;
}
#pragma endregion

#pragma region Process Files
static String PrintDir(File& dir, int numTabs, uint8_t boolIncludeFile)
{
	String buf = "";
	String single = "";
	while (true) {
		single = "";
		File entry = dir.openNextFile();
		if (!entry) {
			// no more files 
			break;
		}
		if (boolIncludeFile == 1 || boolIncludeFile == 2)
		{
			single += entry.fullName();
			single += "\r\n";
		}
		else
		{
			if (entry.isDirectory())
			{
				single += entry.fullName();
				single += "\r\n";
			}
		}
		if (entry.isDirectory() && (boolIncludeFile == 1 || boolIncludeFile == 0)) {
			String d = PrintDir(entry, numTabs + 1, boolIncludeFile);
			delay(10);
			d.trim();
			if (d != "") {
				single += "\r\n/" + d;
			}
		}
		else if (boolIncludeFile == 1 || boolIncludeFile == 2)
		{
			time_t cr = entry.getCreationTime();
			time_t lw = entry.getLastWrite();
			struct tm* tmstruct = localtime(&cr);
			String infoFile = "";

			single.trim();
			if (buf != "" || single != "") {
				if (entry.available())
					infoFile += "," + String(entry.size());
				else
					infoFile += ",0";
				infoFile += "," + DateTimeToString(*tmstruct);
				tmstruct = localtime(&lw);
				infoFile += "," + DateTimeToString(*tmstruct);

				single += infoFile;
				single += "\r\n";
			}
		}
		entry.close();
		if (single != "")
		{
			single.trim();
			WSClient_sendMsgLen(single.c_str(), single.length());
		}
	}
	return "";
}
String ProcessFiles(String data)
{
#if USING_SDCARD
		String command = "MK DIR ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			bool state = createDirectory(data);
			return "Done:" + String(state);
		}
		command = "MK FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			//data.trim();
			String name = data.subString(0, data.indexOf(" "));
			data.remove(0, data.indexOf(" ") + 1);
			//data.trim();
			if (SD_INITIAL) {
				if (SD.exists(name)) SD.remove(name);
				SDCard_WriteFile(name, data, "");
			}
			return String(DONE) + ";" + String(data.length());
		}
		command = "#JOIN FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			//data.trim();
			String name = data.subString(0, data.indexOf(" "));
			data.remove(0, data.indexOf(" ") + 1);
			//data.trim();
			if (SD_INITIAL) {
				//if (SD.exists(name)) SD.remove(name);
				SDCard_WriteFile(name, data, "");
			}
			return String(DONE) + ";" + String(data.length());
		}
		command = "#WRITE FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			String name = getIndex(data, " ");
			data = trimIndex(data, " ");
			data.trim();
			int lkq = 0;
			if (SD_INITIAL) {
				String kq = preprocessor.Join(data);
				if (name.indexOf("+") >= 0) {
					name = preprocessor.Join(name);
				}
				kq.trim();
				if (kq != "") {
					if (kq.indexOf("\\r\\n") >= 0) kq.replace("\\r\\n", "\r\n");
					SDCard_WriteFile(name, kq, "");
				}
				lkq = kq.length();
			}
			return String(DONE) + ";" + String(lkq);
		}//Func_temp.hos	/Administrator/Default-Project/System/Functions/
		command = "RM FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			if (SD.exists(data)) SD.remove(data);
			return "Removed file:" + data;
		}
		command = "CD ONLY DIR ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			if (data == "*") data = "/";
			File root = SD.open(data);
			if (root)
			{
				root.rewindDirectory();
				boolIncludeFile = 0;
				String v = PrintDir(root,0, boolIncludeFile);
				boolIncludeFile = 1;
				root.close();
				Dbg_Println(v);
				return v;
			}
			else
			{
				return NONE;
			}

		}
		//command = "#READ BIN FILE ";
		//if (data.startsWith(command))
		//{
		//	data.remove(0, command.length());
		//	data.trim();
		//	ReadFileBin(data);
		//	return "";
		//}
		command = "CD ONLY FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			File root = SD.open(data);
			if (root)
			{
				root.rewindDirectory();
				boolIncludeFile = 2;
				String v = PrintDir(root, 0, boolIncludeFile);
				boolIncludeFile = 1;
				Dbg_Println(v);
				return v;
			}
			else
			{
				return NONE;
			}

		}
		command = "CD DIR ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			File myfile = SD.open(data);
			if (myfile)
			{
				myfile.rewindDirectory();
				boolIncludeFile = 1;
				String v = PrintDir(myfile, 0, boolIncludeFile);
				Dbg_Println(v);
				myfile.close();
				return v;
			}
			else
			{
				return NONE;
			}

		}
#else
		String command = "MK FILE ";
		if (data.startsWith(command))
		{
			data.remove(0, command.length());
			data.trim();
			String name = data.subString(0, data.indexOf(" "));
			data.remove(0, data.indexOf(" ") + 1);
			data.trim();
			if (name == "Startup.hos")
				memo.WriteData(memo.AllocationStartup, data);
			return DONE;
		}
#endif
		return NONE;
}
String ReadFileProc(String data)
{
	bool isContinue;
	String command;
	command = "#READ FILE ";
	// Get file name
	data.remove(0, command.length());
	data.trim();
	data.replace(";", "");
	// Get dir
	String dir = data;
#if (READ_FILE_STREAM)
	StreamInf_t StreamData;
	char buffer[STREAM_BUFFER_SIZE];

	StreamData.dataptr = buffer;
	StreamData.pos = 0U;
	StreamData.cnt = 0U;
	StreamData.size = STREAM_BUFFER_SIZE;
	// Send to Websocket server
	String cmd = "Command: #BEGIN FILE, ";
	WSClient_sendMsg(cmd.c_str());

/*					  Send data file sequence			
/	|-------------------------------------------------------|
*	|-------------------->Read 1024 byte------------------->|
*	|<--------------server websocket to Send <--------------|
*	|-------------------------------------------------------|
*/
	do
	{
		isContinue = SDCard_ReadFileStream(&StreamData, dir);
		WSClient_sendMsgLen(StreamData.dataptr,StreamData.cnt);
	} while (isContinue);
	//
	cmd = "Command: #END FILE";
	WSClient_sendMsg(cmd.c_str());
#else
	// Load data from SD Card
	data = SDCard_LoadFile(dir);
	if (data != "")
	{
		// Send to Websocket server
		String a = "Command: #BEGIN FILE, ";
		WSClient_sendMsg(a.c_str());
		WSClient_sendMsg(data.c_str());
		a = "Command: #END FILE";
		WSClient_sendMsg(a.c_str());
		return DONE;
	}
	else
	{
		return "FILE IS EMPTY";
	}
#endif
	return DONE;
}
#pragma endregion

#pragma region FirmWare Common
// Create, Setting, Dispose schedule
String ProcessCommon(String data)
{
	//CREATE(SL,noise,00:11:12,02:12:12,1,SEND LCD(0,1,'ahihi'));
	String command = "CREATE(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String type = getIndex(data, ","); data = trimIndex(data, ",");
		String name = getIndex(data, ","); data = trimIndex(data, ",");
		String st = getIndex(data, ","); data = trimIndex(data, ",");
		String et = getIndex(data, ","); data = trimIndex(data, ",");
		String loop = getIndex(data, ","); data = trimIndex(data, ",");
		String e = getLastIndex(data, ")"); data = trimLastIndex(data, ")");
		CreateSchedule(name, st, et, (loop == "1" ? true : false), e);
		return DONE;
	}
	command = "DISPOSE(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String name = getIndex(data, ")");
		DeleteSchedule(name);
		return DONE;
	}

	command = "CREATE FUNCTION ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		//CREATE FUNCTION getTemplate=DECLARE INT SoA=15;DECLARE INT SoB=16; DECLARE INT SoC=0; SET SoC=SoA+SoB; SM;
		String name = data.subString(0, data.indexOf("="));
		String func = data.substring(data.indexOf("=") + 1);
		String dir = memo.GetDirectoryFunction(memo.CurrentUser, memo.CurrentProject); 
		if (SD.exists(dir + memo.GetFileName(name, "F")))
		{
			SD.remove(dir + memo.GetFileName(name, "F"));
		}
		SDCard_WriteFile(dir + memo.GetFileName(name, "F"), func, "");
		memo.Declare("FUNCTION " + name);
		return DONE;
	}
	command = "CREATE BATCH ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String dir = memo.GetDirectoryBatch(memo.CurrentUser, memo.CurrentProject);
		String name = data.subString(0, data.indexOf("="));
		String func = data.substring(data.indexOf("=") + 1);
		createDirectory(dir);
#if USING_SDCARD
		if (SD_INITIAL) {
			if (SD.exists(dir + name + ".hbat")) SD.remove(name);
			SDCard_WriteFile(dir + name + ".hbat", func, "");
		}
#else
		String batch = memo.ReadData(2);
		batch += "*#" + name + "=" + func + "#@|";
		memo.WriteData(memo.AllocationBatch, batch);
#endif
		return DONE;
	}
	command = "CREATE SCREEN ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String dir = memo.GetDirectoryScreen(memo.CurrentUser, memo.CurrentProject);
		String name = data.subString(0, data.indexOf("="));
		String func = data.substring(data.indexOf("=") + 1);
		createDirectory(dir);
#if USING_SDCARD
		if (SD_INITIAL) {
			if (SD.exists(dir + name + ".hscrn")) SD.remove(name);
			SDCard_WriteFile(dir + name + ".hscrn", func, "");
		}
#else
		String batch = memo.ReadData(2);
		batch += "*#" + name + "=" + func + "#@|";
		memo.WriteData(memo.AllocationBatch, batch);
#endif
		return DONE;
	}
	// comment by toanpv1
	//command = "CREATE WEB SERVER ";
	//if (data.startsWith(command))
	//{
	//	data.remove(0, command.length());
	//	data.trim();
	//	CreateWebServer(data);
	//	//memo.Debug("Created web server with port: " + String(port), true);
	//	return DONE;
	//}
	//command = "CREATE TCP ";
	//if (data.startsWith(command))
	//{
	//	data.remove(0, command.length());
	//	data.trim();
	//	TcpClient tc = memo.CreateTcp(data, "TCP");
	//	preprocessor.memoryEx = memo;
	//	memo.ListTcpClients[tc.Index].client.connect(tc.Host.c_str(), tc.Port);
	//	return "Created and connect to server with tcp: " + data;
	//}
	////CREATE 74HC595(1,0,16,2);
	//command = "CREATE 74HC595(";
	//if (data.startsWith(command)) {
	//	data = trimIndex(data, "(");
	//	//Create74hc595(data, typeSend);
	//	return DONE;
	//}

	return NONE;
}
#pragma endregion

#pragma region ProcessCallCommand

static String CallScreen(String name) {
	name.trim();
	String comm = "";
	String dir = memo.GetDirectoryScreen(memo.CurrentUser, memo.CurrentProject);
	comm = SDCard_ReadFile(dir + name + ".hscrn");
	comm.trim();
	return comm;
}

static String CallBatch(String name) 
{
	String result;
	String dir = memo.GetDirectoryBatch(memo.CurrentUser, memo.CurrentProject);
	name.trim();
	SDCard_helpParseBatchFile(dir + name + ".hbat", &result);
	WSClient_PushFiFoBuffer(result);
	return DONE;
}
// Call data
String ProcessCallCommand(String data)
{
	String command = "CALL BATCH ";
	if (data.startsWith(command))
	{
		data = trimLastIndex(data, " ");
		return CallBatch(data);
	}
	command = "CALL SCREEN ";
	if (data.startsWith(command))
	{
		data = trimLastIndex(data, " ");
		return CallScreen(data);
	}
	command = "CALL FUNCTION ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String dir = memo.GetDirectoryFunction(memo.CurrentUser, memo.CurrentProject); 
		//CREATE FUNCTION getTemplate=DECLARE INT SoA=15;DECLARE INT SoB=16; DECLARE INT SoC=0; SET SoC=SoA+SoB; SM;
		String cmd = SDCard_ReadFile(dir + "Func_" + data + ".hos");
		ProcessBatchData(cmd);
		return DONE;
	}
	if (data.startsWith("CALC "))
	{
		if (data.startsWith("CALC ")) data.remove(0, String("CALC ").length());
		String c = preprocessor.Execute(data);
		return c;
	}
	return NONE;
}
#pragma endregion

#pragma region ProcessSendData
// Send Data
String ProcessSendData(String data) {
	String command = "";

	command = "SEND CSERIAL(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.remove(data.length() - 1, 1);
		data.trim();
		String c = data;
		Serial.write(c.c_str(), 1);
		return c;
	}
	//command = "SEND SERIAL(";
	//if (data.startsWith(command))
	//{
	//	data.remove(0, command.length());
	//	data.remove(data.length() - 1, 1);
	//	data.trim();
	//	if (data.indexOf("'+") >= 0 || data.indexOf("'+'") >= 0 || data.indexOf("+'") >= 0 || (data.indexOf("'") < 0 && data.indexOf("+") >= 0))
	//	{
	//		String c = preprocessor.Join(data);
	//		Serial.print(c);
	//	}
	//	Serial.print(data.subString(1, data.lastIndexOf("'")));
	//	return DONE;
	//}
	command = "SEND SERIAL(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.remove(data.length() - 1, 1);
		data.trim();
		if (data.indexOf("'+") >= 0 || data.indexOf("'+'") >= 0 || data.indexOf("+'") >= 0 || (data.indexOf("'") < 0 && data.indexOf("+") >= 0))
		{
			String c = preprocessor.Join(data);
			Serial.print(c);
			message_uart = c;
		}
		else
		{
			Serial.print(data.subString(1, data.lastIndexOf("'") - 1));
			message_uart = data.subString(1, data.lastIndexOf("'") - 1);
		}
		return DONE;
	}
	command = "SEND ESERIAL(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.remove(data.length() - 1, 1);
		data.trim();
		if (data.indexOf("'+") >= 0 || data.indexOf("'+'") >= 0 || data.indexOf("+'") >= 0 || (data.indexOf("'") < 0 && data.indexOf("+") >= 0))
		{
			String c = preprocessor.Join(data);
			Serial.println(c);
		}
		else
			Serial.println(data.subString(1, data.lastIndexOf("'") - 1));
		return DONE;
	}
	//comment by toanpv1
	//command = "SEND WS ";
	//if (data.startsWith(command))
	//{
	//	data.remove(0, command.length());
	//	data.trim();
	//	String c = preprocessor.Join(data);
	//	for (uint8_t i = 0; i < webSocket.connectedClients(true); i++)
	//	{
	//		webSocket.sendTXT(i, c);                // Thông báo nội dung nhận được cho client
	//	}
	//	return c;
	//}
	command = "SEND CLIENT WS ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String c = preprocessor.Join(data);
		WSClient_sendMsg(c.c_str());
		return DONE;
	}
	command = "SEND WS(";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String ind = getIndex(data, ","); data = trimIndex(data, ",");
		data.remove(data.length() - 1, 1);
		String c = preprocessor.Join(data);
		WSClient_sendMsg(c.c_str());
		return DONE;
	}
	command = "SEND TCP ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String name = data.subString(0, data.indexOf("="));
		data.remove(0, name.length() + 1);
		data.trim();
		String c = preprocessor.Join(data);
		for (uint8_t i = 0; i < memo.ListTcpClients.size(); i++)
		{
			if (name == "ALL") {
				if (memo.ListTcpClients[i].client.connected())
					if (memo.ListTcpClients[i].Type == "TCP")
						memo.ListTcpClients[i].client.println(c);
			}
			else
				if (memo.ListTcpClients[i].client.connected())
					if (memo.ListTcpClients[i].Name == name) {
						memo.ListTcpClients[i].client.println(c);
						break;
					}
		}
		return c;
	}
	//HTTP_API
	command = "CALL API GET ";
	if (data.startsWith(command))
	{
		data.remove(0, command.length());
		data.trim();
		String url = preprocessor.Join(data);
		//data = CallAPI("GET", data, 5000, "", false);		//comment by toanpv1
		return data;
	}
	return NONE;
}
#pragma endregion

#pragma region ProcessWhile
String ProcessWhile(String data)
{
	return NOT_SUPPORT;
}
#pragma endregion

#pragma region Process If
// If command
static String TypeIfLC(String logic, String logicTrue, String logicFalse) {
	bool c = preprocessor.ExecuteExpression(logic);
	if (c)
	{
		if (logicTrue != "NULL" && logicTrue != "") {
			ProcessBatchData(logicTrue);
		}
	}
	else {
		if (logicFalse != "NULL" && logicFalse != "") {
			ProcessBatchData(logicFalse);
		}
	}
	return "Done";
}
static String TypeIfSC(String logic, String logicTrue, String logicFalse) {
	String rs = ReadSerial;
	String re = preprocessor.Join(logic);
	if (rs.indexOf(re) >= 0) {
		ProcessBatchData(logicTrue);
		return DONE;
	}
	else {
		ProcessBatchData(logicFalse);
		return DONE;
	}
}
static String TypeIfSE(String logic, String logicTrue, String logicFalse) {
	String rs = ReadSerial;
	rs.trim();
	String re = preprocessor.Join(logic);
	re.trim();
	if (re == rs) {
		ProcessBatchData(logicTrue);
		return DONE;
	}
	else
	{
		ProcessBatchData(logicFalse);
		return DONE;
	}
}
static String TypeIfEN(String logic, String logicTrue, String logicFalse) {
	if (memo.ExistName(logic)) {
		ProcessBatchData(logicTrue);
		return DONE;
	}
	else
	{
		ProcessBatchData(logicFalse);
		return DONE;
	}
}
static String TypeIfCE(String logic, String logicTrue, String logicFalse) {
	String kq = "";
	if (logic.indexOf("@DATETIME") >= 0) {
		logic.replace("@DATETIME", datetime);
	}
	if (logic.indexOf("@DATE") >= 0)
		logic.replace("@DATE", strdate);

	if (logic.indexOf("@TIME") >= 0) {
		logic.replace("@TIME", strtime);
	}
	if (logic.indexOf("@LTIME") >= 0) {
		logic.replace("@LTIME", String(longTime(strtime)));
	}
	if (logic.indexOf("@LDATE") >= 0)
	{
		String ldate = strdate;
		ldate.replace("-", "");
		logic.replace("@LDATE", ldate);
	}
	String reL = preprocessor.Join(logic.split("==", 0));
	String reR = preprocessor.Join(logic.split("==", 1));
	reL.trim();
	reR.trim();
	if (reL == reR) {
		kq = ProcessBatchData(logicTrue);
		return DONE;
	}
	else
	{
		kq = ProcessBatchData(logicFalse);
		return DONE;
	}
}
String ProcessIf(String data)
{
	String kq = "";
	//if(E,a>b?logic true: logic false)
	if (data.startsWith("IF(") || data.startsWith("if("))
	{
		//IF(SE,'#INFOMATION',{SEND LCD(0,2,'OK')},{SEND LCD(0,2,'NG')})
		data.remove(0, 3);
		String type = getIndex(data, ",");
		data = trimIndex(data, ",");
		String logic = getIndex(data, ",");
		data = trimIndex(data, ",");
		bool cf = false;
		if (data.indexOf(":") != data.lastIndexOf(":")) cf = true;
		String logicTrue = getIndex(data, (cf ? "," : ":"));
		data = trimIndex(data, (cf ? "," : ":"));
		String logicFalse = getLastIndex(data, ")");
		logicTrue.trim();
		logicFalse.trim();
		logicTrue.replace("->", ";"); logicTrue += ";";
		logicFalse.replace("->", ";"); logicFalse += ";";
		//là logic khi kiểm tra Serial nhận đc
		if (type == "SE") {
			return  TypeIfSE(logic, logicTrue, logicFalse);
		}
		//là logic khi kiểm tra Serial nhận đc
		if (type == "SC") {
			return  TypeIfSC(logic, logicTrue, logicFalse);
		}
		//là logic so sánh chuỗi bằng
		if (type == "CE") {
			return TypeIfCE(logic, logicTrue, logicFalse);
		}
		//là logic tồn tại
		if (type == "EN") {
			return TypeIfEN(logic, logicTrue, logicFalse);
		}
		//là logic điều kiện
		if (type == "LC") {
			return TypeIfLC(logic, logicTrue, logicFalse);
		}
		return DONE;
	}
	return NONE;
}
#pragma endregion

#pragma region Process Var Let
String ProcessVarLet(String data)
{
	if (data.startsWith("LET ") || data.startsWith("VAR ")) 
	{
		data = trimIndex(data, " ");
		if (data.startsWith("L")) {
			memo.Let(data);
		}
		else {
			memo.Declare(data);
		}
		return DONE;
	}
	return NONE;
}
#pragma endregion

#if USING_HLW8012
#pragma HLW8012
String HLW8012_WSProc(String data)
{
	String command = "HLW8012";
	data.remove(0, command.length());
	data.trim();

	command = "RESET_E";
	if (data.startsWith(command))
	{
		return HLW8012_ResetEnergy(data);
	}
	
	command = "GET_VERSION";
	if (data.startsWith(command))
	{
		return HLW8012_GetVersion(data);
	}
	return NONE;
}
#pragma endregion
#endif
#pragma region Websoket Handler
const WSCmdProcess WSCmdTable[NUMBER_OF_COMMAND_PROCESS] =
{
	// Process Null -> Done
	{"null", &nullProc},{"RETURN", &ReturnProc},{"//", &CommentProc},{"--", &NoneProc},
#if USING_HLW8012
	// Process HLW8012
	{"HLW8012",&HLW8012_WSProc},
#endif
	// Process Set -> Done
	{"SET ", &SETProc},{"SET(", &SETProc_1},
	// Process delay -> Done
	{"DELAY_MS ", &DelayMSProc},{"DELAY_US ", &DelayUSProc},
	// Process Serial -> Not Done
	{"CLEAR SERIAL", &ClearSerialProc},{"WAIT SERIAL(", &WaitSerialProc},{"AWAIT SERIAL(", &AwaitSerialProc},{"REMOVE SERIAL(", &RemoveSerialProc},
	// Process LoadUI -> Done
	{"LOAD SCREEN ", &LoadScreenProc},{"LOAD UI ", &LoadUIProc},
	// Process DXDIAG -> Done
	{"DXDIAG", &DXDIAG},{"GET SIZE FILE ", &DxdiagGetFileSizeProc},
	// Process Pinmode -> Not Done
	{"DIGITAL READ ", &PinReadDIProc},{"DIGITAL WRITE ", &PinWriteDIProc},{"ANALOG WRITE ", &PinWriteANProc},{"ANALOG READ ", &PinReadANProc},
	// Process LR -> Not Done
	{"LR BATCH", &LrBatchProc},{"LR SCREEN", &LrScreenProc},{"LR STARTUP", &LrStartupProc},{"LR SERIAL", &LrBatchProc},
	{"LR SCHEDULE(", &LrBatchProc},{"LR PINMODE(", &LrBatchProc},{"LR WEBSOCKET", &LrBatchProc},{"LR INTERVAL(", &LrBatchProc},{"LR INDEX ", &LrBatchProc},
	{"LR FROM ", &LrBatchProc},{"LR NAME ", &LrBatchProc},{"LR TYPE ", &LrBatchProc},
	// Process WatchDog -> Done
	{"WDT(", &WatchDogProc},{"SWDT", &SoftWatchDogProc},
	// Process Wifi -> Not Done
	{"REFRESH SSID ", &ProcessWifi},{"DISCONNECT WIFI(", &ProcessWifi},{"RE-CONNECT WIFI", &ProcessWifi},{"CLEAR SSID", &ProcessWifi},
	// Type Scan -> Done
	{"SCAN ", &TypeScan},
	// Process Firmware -> Done
	{"FIRMWARE DOWNLOAD(", &FW_DownloadProc},{"FIRMWARE UPDATE", &FW_UpdateProc},
	// Process Execute -> Not Done
	{"TYPE DEVICE", &GetTypeDevice},{"GET RESET INFO", &ProcessExecute},{"DEEP SLEEP ", &ProcessExecute},{"UPDATE TIME", &UpdateTimeProc},
	{"GET TIME", &ProcessExecute},{"RSSI", &ProcessExecute},{"BSSID", &ProcessExecute},{"CM", &ProcessExecute},{"SM", &ProcessExecute},{"SAVE", &ProcessExecute},
	{"RESET VAC", &ProcessExecute},{"RESET PIN(", &ProcessExecute},{"RESET WS", &ProcessExecute},{"RESET", &ResetProc},{"GET STATE ", &ProcessExecute},
	{"GET ", &ProcessExecute},{"DISPOSE ", &ProcessExecute},{"JOIN ", &ProcessExecute},
	// Process Files -> Done
	{"MK FILE ", &ProcessFiles},
#if USING_SDCARD
	{"#JOIN FILE ", &ProcessFiles},{"MK DIR ", &ProcessFiles},{"#WRITE FILE ", &ProcessFiles},{"RM FILE ", &ProcessFiles},{"CD ONLY DIR ", &ProcessFiles},
	{"#READ FILE ", &ReadFileProc},{"CD ONLY FILE ", &ProcessFiles},
#endif
	// Process Common -> Not Done
	{"CREATE(", &ProcessCommon},{"DISPOSE(", &ProcessCommon},{"CREATE FUNCTION ", &ProcessCommon},{"CREATE BATCH ", &ProcessCommon},
	{"CREATE SCREEN ", &ProcessCommon},{"CREATE WEB SERVER ", &ProcessCommon},{"CREATE TCP ", &ProcessCommon},
	{"SETTING(", &ProcessCommon},
	// Process Call Command -> Not Done
	{"CALL FUNCTION ", &ProcessCallCommand },{ "CALL BATCH ", &ProcessCallCommand },{"CALL SCREEN ", &ProcessCallCommand},{"CALC ", &ProcessCallCommand},
	// Process Send Data -> Not Done
	{"SEND CSERIAL(", &ProcessSendData},{"SEND SERIAL(", &ProcessSendData},
	{"SEND ESERIAL(", &ProcessSendData},{"SEND WS ", &ProcessSendData},{"SEND CLIENT WS ", &ProcessSendData},{"SEND WS(", &ProcessSendData},
	{"SEND TCP ", &ProcessSendData},{"CALL API GET ", &ProcessSendData},
	// Process While -> Not Done
	{"WHILE(", &ProcessWhile},{"while(", &ProcessWhile},{"AWHILE(", &ProcessWhile},{"awhile(", &ProcessWhile},
	// Process If -> Done
	{"IF(", &ProcessIf},{"if(", &ProcessIf},
	// Process Var Let -> Done
	{"LET ", &ProcessVarLet},{"VAR ", &ProcessVarLet},
};

// Xử lý cho các ngoại lệ khi nhận gói lệnh từ server
void WSHandleException(String* message)
{
	if ((*message).startsWith("CREATE FUNCTION") || (*message).startsWith("MK FILE "))
	{
		ProcessFiles(*message);
		*message = "";
	}
	else
	{
		int idx;
		(*message).replace("\r", "");
		if (((*message).startsWith("CALL BATCH ")||("RM FILE ")) && !(*message).endsWith(";")) {
			(*message) += ";";
		}

		while ((*message).indexOf("\n") >= 0)
		{
			idx = (*message).indexOf("\n");
			String cmd = (*message).subString(0, idx + 1);
			WSClient_PushFiFoBuffer(cmd);
			(*message).remove(0, idx + 1);
		}
	}
}

void WSHandle_Init()
{
	// Khai báo xử lý ngoại lệ cho bản tin websocket
	WSClient_onHandleException(WSHandleException);
}

bool WSHandle_Main()
{
	static unsigned long time = millis();
	String data = "";
	String cmd = "";
	String dataSend = "";
	String ret = NONE;
	// Lưu allocate vào thẻ nhớ
	if ((memo.isSM == true) && (millis() - time > memo.SM_Interval))
	{
		memo.SaveMemory();
		time = millis();
		memo.isSM = false;
	}
	// Lấy data nhận được từ Websocket buffer
	data = WSClientGetCommand();
	if (data == "") return 0;
	data.trim();
	cmd = getIndex(data, ";");

	Dbg_Printf("WebSocket Cmd: ");
	Dbg_Println(cmd);
	data = cmd;
	// Kiểm tra trong bảng lệnh. Nếu khớp thì thực hiện lệnh xử lý tương ứng
	for (uint8_t i = 0; i < NUMBER_OF_COMMAND_PROCESS; i++)
	{
		if (data.startsWith(WSCmdTable[i].cmd))
		{
			ret = WSCmdTable[i].process(data);
			dataSend = "\r\nCommand: " + cmd + ", process: " + ret;
			WSClient_sendMsg(dataSend.c_str());
			return 1;
		}
	}
	return 0;
}
#pragma endregion