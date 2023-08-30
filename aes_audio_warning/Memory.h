
// 
/*
	Name:       Wifi_Testing.ino
	Created:	21/05/2021 5:53:50 CH
	Author:     HomeOS VietNam .Jsc, hung.pham
*/

// Define User Types below here or use a .h file
// Memory.h

#ifndef _MEMORY_h
#define _MEMORY_h 
#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.hpp>
#include <ArduinoJson.h>
#include "LinkedList.h"
#include "WiFiClient.h"

#define MAX_VARIABLE					(50U)
#define MAX_LENGTH_VARIABLE				(28U)

typedef enum {
	BOOL,							// nhị phân => cấp 1 ô nhớ
	BYTE,							// Số nguyên => cấp phát 3 ô nhớ
	INT,							// Số nguyên => cấp phát 5 ô nhớ
	DATE,							// lưu trữ theo ISO, cấp 10 ô nhớ, yyyy-MM-dd
	DATETIME,						//lưu trữ theo ISO, cấp 18 ô nhớ, yyyy-MM-dd hh:mm:ss
	TIME,							// cấp 8 ô nhớ, hh:mm:ss
	VAR,							//Cấp phát tuỳ biến
	TCPCLIENT,						//Cấp phát tuỳ biến
	SOCKETCLIENT,					//Cấp phát tuỳ biến
	FTPCLIENT,						//Cấp phát tuỳ biến
	TCPSERVER,						//Cấp phát tuỳ biến
	FUNCTION,						//Cấp phát tuỳ biến
	THREAD,							//Cấp phát tuỳ biến
	TIMER,							//Cấp phát tuỳ biến
	WIFI,							//Cấp phát tuỳ biến
	TABLE,							//Cấp phát tuỳ biến
	DATABASE,						//Cấp phát tuỳ biến
	PROJECT,						//Cấp phát tuỳ biến
	USER,							//Cấp phát tuỳ biến
	PIN_IN,							//Cấp phát tuỳ biến
	PIN_OUT,						//Cấp phát tuỳ biến
	NUMBER							// Số thập phân => cấp phát 8 ô nhớ
} MemoryType;
typedef struct {
	uint	BOOL = 1;               //Add:01. Nhị phân => cấp 1 ô nhớ
	uint	BYTE = 3;				//Add:02. Số nguyên => cấp phát 3 ô nhớ
	uint	INT = 5;				//Add:03. Số nguyên => cấp phát 5 ô nhớ
	uint	DATE = 10;				//Add:17. lưu trữ theo ISO, cấp 10 ô nhớ, yyyy-MM-dd
	uint	DATETIME = 19;			//Add:18. lưu trữ theo ISO, cấp 18 ô nhớ, yyyy-MM-dd hh:mm:ss
	uint	TIME = 8;				//Add:19. cấp 8 ô nhớ, hh:mm:ss
	uint	VAR = 1;				//Add:20. cấp phát tuỳ biến
	uint	NUMBER = 3;				//Add:20. cấp 3 ô nhớ
	uint	TCPCLIENT = 64;			//Add:20. cấp 32 ô nhớ
	uint	SOCKETCLIENT = 64;		//Add:20. cấp 32 ô nhớ
	uint	TCPSERVER = 32;			//Add:20. cấp phát tuỳ biến
	uint	FTPCLIENT = 32;			//Add:20. cấp 32 ô nhớ
	uint	FUNCTION = 1;			//Add:20. cấp phát tuỳ biến
	uint	THREAD = 16;			//Add:20. cấp phát tuỳ biến
	uint	TIMER = 16;				//Add:20. cấp phát tuỳ biến
	uint	WIFI = 64;				//Add:20. cấp phát tuỳ biến
	uint	TABLE = 1;				//Add:20. cấp phát tuỳ biến
	uint	PROJECT = 1;			//Add:20. cấp phát tuỳ biến
	uint	USER = 1;				//Add:20. cấp phát tuỳ biến
	uint	PIN_IN = 1;				//Add:20. cấp phát tuỳ biến
	uint	PIN_OUT = 1;			//Add:20. cấp phát tuỳ biến
	uint	DATABASE = 1;			//Add:20. cấp phát tuỳ biến
} MemorySize;

typedef struct {
	uint8_t Index = 0;					//Thứ tự trong danh sách
	String Name = "";					//Tên biến
	String Host = "";					//Địa chỉ kết nối tới dịch vụ
	uint Port = 3388;					//Cổng kết nối tới dịch vụ
	uint Timeout = 5;					//Thời gian tính là timeout nếu ko có phản hồi
	//String Header = "";				//Thời gian tính là timeout nếu ko có phản hồi
	String Type = "TCP";				//Kiểu truyền thông TCP/HTTP/FTP/UDP
	WiFiClient client;
} TcpClient;
//typedef struct {
//	String Name = "";
//	String Command = "";
//	uint Interval = 0;
//	uint CountDown = 0;
//	bool isTimeout = false;
//} TimerExecute;
//typedef struct {
//	String Name = "";
//	String Func = "";
//	//0 -> PULLUP
//	//1 -> PULLDOWN16
//	//2 -> NORMAL
//	uint8_t Type = 0;
//	ulong Millis = 0;
//	ulong CounterSummation = 0;
//	long CounterSubtraction = 0;
//	uint8_t Pin = 0;
//	bool isOut = false;
//	bool Value = false;
//	bool isResetSummation = false;
//	bool isResetSubtraction = false;
//	bool hasInterupt = false;
//	//1 RISING to trigger when the pin goes from LOW to HIGH.
//	//2 FALLING for when the pin goes from HIGH to LOW;
//	//3 CHANGE to trigger the interrupt whenever the pin changes value – for example from HIGH to LOW or LOW to HIGH;
//	//4 ONLOW to trigger when the pin goes from LOW to HIGH.
//	//5 ONHIGH to trigger when the pin goes from LOW to HIGH.
//	uint8_t typeInterupt = 0;
//} PinRuntime;

//typedef struct {
//	int Timer = 0;
//	String Name = "";
//	String Start_Time = "";
//	String End_Time = "";
//	bool isloop = false;
//	String Event = "";
//} Scheduling;

typedef struct {
	uint Page /*                       */ = 0;			        //Trang nhớ
	uint Address /*                    */ = 0;					// Địa chỉ vùng nhớ thực tế
	String Name /*                     */ = "";					//Tên biến
	MemoryType Type /*                 */ = MemoryType::VAR;	// Kiểu dữ liệu
	bool isNegative /*                 */ = false;			    //Nếu là kiểu số, thì là số âm hay dương, mặc định là dương => false
	String Value /*                    */ = "";					// Giá trị lưu trữ
	uint Size;
	String DecimalPlace /*             */ = "";
	bool isRam /*                      */ = false;			    // nếu biến khai báo chỉ tạo trên RAM, khi khởi động lại thì xoá, ngược lại thì lưu trữ vào flash
} AllocateAddress;
class MemoryClass
{
private:
	uint indexDebug = 0;

	//Dung lượng vùng nhớ, với 50*28=1400 ô nhớ đầu để phân bổ cho địa chỉ 4096 dành cho dữ liệu.
	uint MAX_SIZE = 1024;//MAX_LENGTH_VARIABLE * MAX_VARIABLE + (4096 - MAX_LENGTH_VARIABLE * MAX_VARIABLE);

	//giá trị biến lưu dạng mảng - dành cho flash
	String ValueOnFlash /*                                                 */ = "";
	//Bảng địa chỉ phân bổ phục vụ tra cứu
	//[Tên biến 16 kí tự][page 2 kí tự][kiểu khai báo 2 kí tự][địa chỉ vùng nhớ thực tế 4 kí tự] => 24 kí tự
	String AllocateOnFlash /*                                              */ = "";
	//Địa chỉ được cấp tính tới thời điểm hiện tại.
	uint LastAddress /*                                                    */ = 0;
	//Địa chỉ được cấp tính tới thời điểm hiện tại.
	uint LastAllocateData /*                                               */ = 0;


protected:
	void(*_onListen)();
	void(*_onEvent_Saved)();
	String(*_onExpression)(String data);
	void(*_onLoadMemory)();
	void(*_onSaveMemory)(DynamicJsonDocument djd);
	void(*_onClearMemory)();
	void(*_onSaveToSD)(String name, String value, MemoryType memo_type);
	String(*_onGetValueFromSD)(String name);
	String(*_onGetValue)(AllocateAddress aa);
	void(*_onEvent_DataProcessed)(String command, String data, String typeSending, String format);
public:
	//Vị trí phân bổ khai báo 1024 bytes
	uint8_t AllocationVariable = 0;
	//Vị trí phân bổ nội dung Startup 1024 bytes
	uint8_t AllocationStartup = 1;
	//Vị trí phân bổ các Batch 1024 bytes
	uint8_t AllocationBatch = 2;
	//Ví trị phân bổ các logger 1024 bytes
	uint8_t AllocationLogger = 3;
	//User hiện tại đang kích hoạt ws
	String CurrentUser /*                          */ = "Administrator";
	//Dự án hiện tại đang kích hoạt
	String CurrentProject /*                       */ = "Default-Project";
	//Json lưu trữ phân bổ khai báo biến
	DynamicJsonDocument* documentAllocate;
	String cacheConvertMemoryRAM /*                */ = "";
	String  StartupFile /*                         */ = "{u}/{p}/";//Startup.hos
	String  LoopFile /*                            */ = "{u}/{p}/";//Startup.hos
	//Định dạng thư mục chứa hàm
	String dirFunction /*                          */ = "{u}/{p}/System/Functions/";
	String dirBatch /*                             */ = "{u}/{p}/System/Batchs/";
	String dirScreen /*                            */ = "{u}/{p}/System/Screens/";
	//Định dạng thư mục chứa chuỗi
	String dirString /*                            */ = "{u}/{p}/System/String/";
	//Định dạng thư mục chứa dữ liệu logger
	String dirLogger /*                            */ = "{u}/{p}/Data/Logger/";
	//Cơ sở dữ liệu của project
	String dirDatabase /*                          */ = "{u}/{p}/Data/Database/";
	//Giao diện thiết kế
	String dirUI /*                                */ = "{u}/{p}/UI/Design/";
	//Giao diện các runtime
	String dirUIRuntime /*                         */ = "{u}/{p}/UI/Runtime/";
	bool ENABLE_DEBUG /*                           */ = true;
	bool isSM		  /*                           */ = false;
	uint16_t SM_Interval = 6000U;
	//Hoạt động khi sử dụng thẻ SD
	bool USING_SD /*                               */ = true;
	//Nội dung ghi nhận chỉ dành cho truyền tin
	String ContentMessage /*                       */ = "";	
	//******************DEFINE PROPERTY******************//
	//Khởi tạo đối tượng Memory;
	void initial(size_t size);
	//xoá dữ liệu theo vị trí
	void ClearStructValue(uint index);
	//Khởi tạo đối tượng Memory mặc định 4096 bytes;
	void initial();
	void ResetAllocateAddress();
	//Xoá toàn bộ bộ nhớ
	void ClearAllMemory();

	String GetVariable(uint i);
	//Vị trí lưu trữ biến
	AllocateAddress ListAllocateAddress[MAX_VARIABLE];
	//Vị trí lưu trữ biến
	LinkedList<TcpClient> ListTcpClients = LinkedList<TcpClient>();
	//******************DEFINE PROTOTYPE METHODS******************//
	//Lấy TcpClients theo name
	TcpClient GetTcpClientByName(String name);
	//Tải tất cả biến lên RAM
	void LoadMemory();
	//
	String SerializeAllocate();
	//
	DynamicJsonDocument DeserializeAllocate();
	//Lấy dữ liệu vùng nhớ hiện tại
	String GetMemoryDataCurrent();
	//Lấy dữ liệu vùng nhớ hiện tại
	String SetMemoryDataCurrent(String data);
	//Lấy dữ liệu vùng nhớ hiện tại
	String GetAllocateMemoryDataCurrent();
	//Lấy dữ liệu vùng nhớ hiện tại
	String SetAllocateMemoryDataCurrent(String data);
	//Định dạng lại kết quả xuất ra hoặc lưu vào bộ nhớ
	String FormatValue(String name, double value);
	//Trả về độ rộng kiểu dữ liệu theo kiểu
	uint GetSizeVariableType(MemoryType type);
	//trả về độ rộng kiểu dữ liệu theo địa chỉ
	uint GetSizeVariableAddress(uint8_t add);
	//Trả về địa chỉ biến theo kiểu dữ liệu
	uint GetAddressByType(MemoryType type);
	MemoryType GetTypeByAddress(uint add);
	//Giá trị mặc định của kiểu khai báo
	String DefaultValue(MemoryType type);
	//Lệnh khởi tạo biến
	bool Declare(String data);
	//Lệnh khởi tạo biến chỉ khai báo trên RAM
	bool Let(String data);
	bool Set(String data);
	String* Get(String data);
	//Lệnh khởi tạo biến
	bool CommandCreateVar(String data, bool isram);	//Tạo khai báo biến
	//Tạo khai báo biến
	bool CreateVariable(String name, MemoryType type, String size, String value, bool isram);
	//Tạo dữ liệu TcpClient
	TcpClient CreateTcp(String data, String type);
	//Trả về struct của biến theo tên
	AllocateAddress GetPropertiesByName(String name);
	//Huỷ biến
	bool Dispose(String name);
	//Tìm chọn địa chỉ phân bổ sẵn sàng cho biến
	uint AvailableAllocateAddress();
	//kiểm tra sự tồn tại của biến
	bool ExistName(String name);
	//Lấy giá trị ra
	String* GetValue(String name);
	//Xuất giá trị sang dạng Int
	uint toInt(String name);
	//Chuyển sang dạng float
	float toFloat(String name);
	//Chuyển sang dạng Double
	double toDou(String name);
	//Chuyển sang dạng năm
	String toYear(String name);
	//Chuyển sang dạng tháng
	String toMonth(String name);
	//Chuyển sang dạng ngày
	String toDay(String name);
	//Chuyển sang dạng giờ
	String toHour(String name);
	//Chuyển sang dạng phút
	String toMinute(String name);
	//Chuyển sang dạng giây
	String toSecond(String name);
	//Kiểm tra xem có phải là số không
	bool isNumberic(String name);
	//Đặt giá trị vào biến
	bool SetValue(String name, String value);
	//Xoá dữ liệu biến
	bool ClearValue(String name);
	//trả về ô nhớ đc phân bổ cuối cùng
	uint16_t LastAllocate();
	//Lấy địa chỉ cuối cùng trong mảng chứa giá trị
	uint GetLastAddress();
	//Trả về độ rộng tổng vùng phân bổ
	uint16_t length();
	//Lưu dữ liệu vào flash
	void SaveMemory();
	// hoạt động như 1 timer
	void onListen(void(*callback)());
	//Sự kiện xảy ra khi lưu dữ liệu vào flash
	void onEvent_Saved(void(*callback)());
	//Sự kiện xảy ra khi 1 biểu thức điều kiện hoạt động
	void onExpression(String(*callback)(String data));
	void onSaveMemory(void(*callback)(DynamicJsonDocument djd));
	void onLoadMemory(void(*callback)());
	void onClearMemory(void(*callback)());
	//Sự kiện lưu dữ liệu vào thẻ nhớ đối với VAR
	void onSaveToSD(void(*callback)(String name, String value, MemoryType memo_type));
	//Sự kiện trả về dữ liệu từ thẻ nhớ đối với FUNCTION và VAR
	void onGetValueFromSD(String(*callback)(String name));
	void onGetValue(String(*callback)(AllocateAddress aa));
	DynamicJsonDocument ReadFileJSON(String fileName);
	void onEvent_DataProcessed(void(*callback)(String command, String data, String typeSending, String format));
	//Mã hoá
	String Encode(String strInput);
	//Lấy cấu trúc thư mục Startup
	String GetStartup(String user, String project);
	//Lấy cấu trúc thư mục Loop
	String GetLoop(String user, String project);
	//Lấy cấu trúc thư mục logger
	String GetDirectoryLogger(String user, String project);
	//Lấy cấu trúc thư mục Batch
	String GetDirectoryBatch(String user, String project);
	//Lấy cấu trúc thư mục Screen
	String GetDirectoryScreen(String user, String project);
	//Lấy cấu trúc thư mục String
	String GetDirectoryString(String user, String project);
	//Lấy cấu trúc thư mục Function
	String GetDirectoryFunction(String user, String project);
	//Lấy tên file theo cấu trúc
	String GetFileName(String name, String type);
	//Tạo user 
	String GetAllocateFileName(String user, String project);
	//
	void SetCurrentUser(String user, String project);
};
extern MemoryClass memo;
#endif

