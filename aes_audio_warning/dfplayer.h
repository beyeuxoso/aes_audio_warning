#ifndef __AES_DFPLAYER_H_
#define __AES_DFPLAYER_H_

#include "Preprocess.h"

#define NUMBER_OF_COMMAND_PROCESS_UART			(8U)

typedef String(*ProcessCmmd)(String data);

typedef struct
{
	String cmd;
	ProcessCmmd process;
} UartCmdProcess;

extern void dfplayer_init();
extern bool UartHandle_Main();

#endif	/* __AES_SERIAL_HANDLE_H_ */