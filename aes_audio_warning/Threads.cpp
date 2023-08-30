
#include "Threads.h"
#include "Thread.h"
#include "ThreadController.h"
#include "Dbg.h"

// Include
#include "WSClient.h"
#include "AppWiFi.h"
#include "RTC_DS3231.h"
#include "ntp.h"
#include "WSHandle.h"

// Dành cho quản lý các Thread
ThreadController controller = ThreadController();

// Luồng cho kiểm tra kết nối WiFi
Thread ThreadWiFiCheckConnect = Thread();

// Luồng cho đọc thời gian thực
Thread ThreadRTC = Thread();

// Luồng cho chạy các sự kiện IV, TO...
Thread ThreadTimeExcute = Thread();

// Luồng cho chạy các tác vụ lập lịch
Thread ThreadSchedule = Thread();

// Luồng cho in log test
Thread ThreadCheckWS = Thread();

// Luồng cho in log test
Thread ThreadTest = Thread();

void Test_Main()
{
	//digitalWrite(LED_DEBUG, !digitalRead(LED_DEBUG));
	Dbg_Printf("Heap remain: %d\n", ESP.getFreeHeap());
}

void Threads_Init() {
#if 1
	ThreadWiFiCheckConnect.enabled = true;
	ThreadWiFiCheckConnect.setInterval(10000);
	ThreadWiFiCheckConnect.onRun(AppWiFi_Main);
	controller.add(&ThreadWiFiCheckConnect);
#endif

#if 1
	ThreadRTC.enabled = true;
	ThreadRTC.setInterval(1000);
	ThreadRTC.onRun(RTC_DS3231_Main);
	controller.add(&ThreadRTC);
#endif

#if 1
	ThreadTimeExcute.enabled = true;
	ThreadTimeExcute.setInterval(100);
	ThreadTimeExcute.onRun(ExecuteTimeProc_Main);
	controller.add(&ThreadTimeExcute);
#endif

#if 1
	ThreadSchedule.enabled = true;
	ThreadSchedule.setInterval(450);
	ThreadSchedule.onRun(ExecuteSchedule_Main);
	controller.add(&ThreadSchedule);
#endif

#if 1
	ThreadCheckWS.enabled = true;
	ThreadCheckWS.setInterval(WS_CHECK_ALIVE_INTERVAL);
	ThreadCheckWS.onRun(WSClient_CheckAlive);
	controller.add(&ThreadCheckWS);
#endif

#if 1
	ThreadTest.enabled = true;
	ThreadTest.setInterval(1000);
	ThreadTest.onRun(Test_Main);
	controller.add(&ThreadTest);
#endif
}

void Threads_Run() {
	controller.run();
}
