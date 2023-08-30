#include "serial_handle.h"
#include "WSHandle.h"
#include "Dbg.h"

uint16_t bytesRead = 0;
String message_uart = "";
char data[1024];

FIFO UartRxBuffer(RX_BUFFER_LEN);

void (*_Uart_onHandleException)(String* message);

void Uart_onHandleException(void (*callback)(String* message))
{
	_Uart_onHandleException = callback;
}

void Uart_helpPushFiFoBuffer(String message)
{
    //// Skip comment
    //if (message.startsWith("//"))
    //{
    //    return;
    //}
    //// Check message is valid or not?
    //if ((message.length() > WS_MESSAGE_LEN_MAX) || (message[message.length() - 1] != ';'))
    //{
    //    Dbg_Printf("Serial message recived is not valid: ");
    //    Dbg_Println(message);
    //}
    //else {
        for (uint8_t i = 0; i < message.length(); i++)
        {
            if (!UartRxBuffer.isFull()) {
                UartRxBuffer.push(message[i]);
            }
            else {
                Dbg_Printf("Serial is full. Skipping... \r\n");
            }
        }
   // }
}

void Uart_PushFiFoBuffer(String multiMsg)
{
    String command;
    //int pos = 0;
    //while ((pos = multiMsg.indexOf(';')) != -1)
    //{
        // Trích xu?t chu?i con t? ??u ??n v? trí c?a d?u ';'
        //command = multiMsg.substring(0, pos + 1);
        //command.trim();
        // Xóa chu?i con ?ã ???c trích xu?t và d?u ';'
        //multiMsg.remove(0, pos + 1);
        //Uart_helpPushFiFoBuffer(command);
        Uart_helpPushFiFoBuffer(multiMsg);
    //}
}



String SerialGetCommand()
{
    String cmd = "";
    bool pause = false;
    while ((!UartRxBuffer.isEmpty()) && (!pause))
    {
        char chr = UartRxBuffer.pop();
        cmd += chr;
        if (chr == ';') {
            pause = true;
        }
    }
    return cmd;
}

void Uart_Event() {  
        if (_Uart_onHandleException != NULL)
        {
            _Uart_onHandleException(&message_uart);
        }
        if (message_uart != "")
        {
            Uart_PushFiFoBuffer(message_uart);
        }
        message_uart = "";
        memset(data, '\0', bytesRead);
        bytesRead = 0;    
}
