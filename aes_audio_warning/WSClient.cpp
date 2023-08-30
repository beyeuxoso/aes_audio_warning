
#include <WebSocketsServer.h>           // Thư viện WebSocketsServer
#include <WebSocketsClient.h>

#include "WSClient.h"
#include "Dbg.h"
#include "Dxdiag.h"
#include "Memory.h"
//Từ khoá để dừng chế độ Websocket
const String KeywordWS_Break = "3088187e-2218-427c-bd5d-424b808e4452";

static bool WSClient_IsAlive = false;
static uint8_t WSClient_IsAliveCnt = 0;
static uint8_t WSClient_PingCnt = 0;
static uint8_t WSClient_Timeout = WS_PING_INTERVAL_30S;

WebSocketsClient WSClient;
FIFO WSClientRxBuffer(WS_RX_BUFFER_LEN);

void (*_WSClient_onHandleException)(String* message);

void WSClient_onHandleException(void (*callback)(String* message))
{
	_WSClient_onHandleException = callback;
}
static void WSClientSetAlive()
{
	WSClient_IsAlive = true;
	WSClient_IsAliveCnt = 0;
}
static void WSClient_GetInfo(WSClientInf_t* wsInfo)
{
	for (uint8_t i = 0; i < MAX_VARIABLE; i++)
	{
		if (memo.ListAllocateAddress[i].Name == "") continue;
		if (memo.ListAllocateAddress[i].Name != "" && memo.ListAllocateAddress[i].Type == MemoryType::SOCKETCLIENT)
		{ 
			String data = memo.ListAllocateAddress[i].Value;
			String ptc = data.subString(0, data.indexOf(":"));

			data.remove(0, ptc.length() + 3);
			wsInfo->host = data.subString(0, data.indexOf(":"));

			data.remove(0, wsInfo->host.length() + 1);
			wsInfo->port = data.subString(0, data.indexOf("/"));

			data.remove(0, wsInfo->port.length());
			wsInfo->path = data;
			wsInfo->path.trim();

			Dbg_Printf("[Websocket Client Info] Host: ");
			Dbg_Print(wsInfo->host);
			Dbg_Print(", port: ");
			Dbg_Print(wsInfo->port);
			Dbg_Print(", path: ");
			Dbg_Println(wsInfo->path);
		}
	}
}
static void WSClientPing(uint8_t interval = WS_PING_INTERVAL_20S)
{
	String s = "PING/" + String(ESP.getChipId()) + "/" + String(interval);
	bool val = WSClient.sendPing(s);
}

void WSClient_Init()
{
	WSClientInf_t wsInfo;
	WSClient_GetInfo(&wsInfo);
	WSClient.begin(wsInfo.host, wsInfo.port.toInt(), wsInfo.path);
	WSClient.onEvent(WSClient_Event);
	WSClient.setReconnectInterval(500);
}

void WSClient_ReInit()
{
	WSClient.disconnect();
	WSClient_Init();
}

void WSClient_ReConnect()
{
	if (!WSClient.isConnected())
	{
		WSClientInf_t wsInfo;
		WSClient_GetInfo(&wsInfo);
		WSClient.begin(wsInfo.host, wsInfo.port.toInt(), wsInfo.path);
		WSClient.onEvent(WSClient_Event);
	}
}

void WSClient_sendMsg(const char* msg)
{
	WSClient.sendTXT(msg);
}

void WSClient_sendMsgLen(const char* msg, size_t len)
{
	WSClient.sendTXT(msg,len);
}

void WSClient_Main()
{
	WSClient.loop();
}

void WSClient_Event(WStype_t type, uint8_t* payload, size_t length) {
	String message = "";
	switch (type) {
	case WStype_TEXT:
		message = (char*)payload;
		// Thông báo nội dung nhận được cho server
		WSClient_sendMsg(message.c_str());
		// Xử lý các trường hợp đặc biệt như: Make File, Make Function, 
		// loại bỏ comment trong gói lệnh nhận được từ server.
		if (_WSClient_onHandleException != NULL)
		{
			_WSClient_onHandleException(&message);
		}
		if (message != "")
		{
			WSClient_PushFiFoBuffer(message);
		}
		break;
	case WStype_PING:
		Dbg_Println("Ping received");
		break;
	case WStype_PONG:
		WSClientSetAlive();
		Dbg_Println("Pong received");
		break;
	case WStype_DISCONNECTED:
		Dbg_Printf("WSClient disconnected.\r\n");
		break;
	case WStype_CONNECTED:
		WSClientSetAlive();
		WSClientPing(WS_PING_INTERVAL_20S);
		Dbg_Printf("WSClient connected.\r\n");
		break;
	case WStype_BIN:
		//Dbg_Printf("[WSc] get binary length: %u\n", length);
		break;
	}
}

bool WSClient_CheckConnect()
{
	bool ret = false;
	ret = WSClient.isConnected();
	return ret;
}

String WSClientGetCommand()
{
	String cmd = "";
	bool pause = false;
	while ((!WSClientRxBuffer.isEmpty()) && (!pause))
	{
		char chr = WSClientRxBuffer.pop();
		cmd += chr;
		if (chr == ';') {
			pause = true;
		}
	}
	return cmd;
}

void WSClient_helpPushFiFoBuffer(String message)
{
	// Skip comment
	if (message.startsWith("//"))
	{
		return;
	}
	// Check message is valid or not?
	if ((message.length() > WS_MESSAGE_LEN_MAX) || (message[message.length() - 1] != ';'))
	{
		Dbg_Printf("Websocket message recived is not valid: ");
		Dbg_Println(message);
	}
	else {
		for (uint8_t i = 0; i < message.length(); i++)
		{
			if (!WSClientRxBuffer.isFull()) {
				WSClientRxBuffer.push(message[i]);
			}
			else {
				Dbg_Printf("WSClientRxBuffer is full. Skipping... \r\n");
			}
		}
	}
}

void WSClient_PushFiFoBuffer(String multiMsg)
{
	String command;
	int pos = 0;
	while ((pos = multiMsg.indexOf(';')) != -1)
	{
		// Trích xuất chuỗi con từ đầu đến vị trí của dấu ';'
		command = multiMsg.substring(0, pos + 1);
		command.trim();
		// Xóa chuỗi con đã được trích xuất và dấu ';'
		multiMsg.remove(0, pos + 1);
		WSClient_helpPushFiFoBuffer(command);
	}
}

void WSClient_CheckAlive()
{
	WSClient_PingCnt++;
	if (WSClient_PingCnt > WSClient_Timeout)
	{
		String s = "PING/" + String(ESP.getChipId());
		bool val = WSClient.sendPing(s);
		WSClient_PingCnt = 0;
	}

	WSClient_IsAliveCnt++;
	if (WSClient_IsAliveCnt > WSClient_Timeout)
	{
		WSClient_IsAlive = false;
		
	}
	if (WSClient_IsAlive == false)
	{
		Dbg_Println("Websoket connection is not alive -> Reset websoket.");
		//WSClient_ReInit();
	}
}