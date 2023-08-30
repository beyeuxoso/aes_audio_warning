#ifndef __AES_WEBSOCKET_CLIENT_H_
#define __AES_WEBSOCKET_CLIENT_H_

#include <WebSocketsServer.h>           // Thư viện WebSocketsServer
#include <WebSocketsClient.h>
#include "FIFO.h"

#define AES_WSKEY						"6700f512-e0ce-4ff3-b1b7-8956b166e48f"
#define WS_MESSAGE_LEN_MAX				(0xFFU)
#define WS_RX_BUFFER_LEN				(1024U)
#define WS_CHECK_ALIVE_INTERVAL			(1000U)

#define WS_PING_INTERVAL_10S			(10U)
#define WS_PING_INTERVAL_20S			(20U)
#define WS_PING_INTERVAL_30S			(30U)
#define WS_PING_INTERVAL_DEFAULT_60S	(60U)

typedef struct
{
	String host;
	String port;
	String path;
} WSClientInf_t;

extern FIFO WSClientRxBuffer;
extern void WSClient_Event(WStype_t type, uint8_t* payload, size_t length);

extern void WSClient_Init();
extern void WSClient_ReInit();
extern void WSClient_Main();
extern void WSClient_sendMsg(const char* msg);
extern void WSClient_sendMsgLen(const char* msg, size_t len);
extern String WSClientGetCommand();
extern bool WSClient_CheckConnect();
extern void WSClient_PushFiFoBuffer(String multiMsg);
extern void WSClient_onHandleException(void (*callback)(String* message));
extern void WSClient_CheckAlive();
#endif	/* __AES_WEBSOCKET_CLIENT_H_ */