
#ifndef __WEBSOCKET_HANDLE_H_
#define __WEBSOCKET_HANDLE_H_

#include "WSClient.h"

#include "Preprocess.h"

#define NUMBER_OF_COMMAND_PROCESS			(96U)

typedef String(*ProcessCmd)(String data);

typedef struct
{
	String cmd;
	ProcessCmd process;
} WSCmdProcess;
extern void WSHandle_Init();
extern bool WSHandle_Main();
extern void ExecuteTimeProc_Main();
extern void ExecuteSchedule_Main();
extern void Memory_Initial();
extern void Preprocess_Init();

extern PreprocessClass preprocessor;
extern String ProcessBatchData(String data);
#endif /* __WEBSOCKET_HANDLE_PROCESS_H_ */