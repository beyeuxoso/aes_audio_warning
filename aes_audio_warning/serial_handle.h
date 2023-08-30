#ifndef __AES_SERIAL_HANDLE_H_
#define __AES_SERIAL_HANDLE_H_

#include "FIFO.h"

#define RX_BUFFER_LEN				(1024U)

extern FIFO UartRxBuffer;
extern String message_uart;
extern void Uart_PushFiFoBuffer(String multiMsg);
extern String SerialGetCommand();
extern void Uart_Event();
extern void Uart_onHandleException(void (*callback)(String* message));

#endif	/* __AES_SERIAL_HANDLE_H_ */