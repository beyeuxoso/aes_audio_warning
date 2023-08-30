#include "dfplayer.h"
#include "serial_handle.h"
#include "Dbg.h"
#include "HelpString.h"
#include "DFRobotDFPlayerMini.h"

DFRobotDFPlayerMini myMP3;


void dfplayer_init() {
    myMP3.begin(audio, false);
    myMP3.volume(20);
}

String play(String data) {
    data.remove(0, 4);
    data.trim();
    int track = data.toInt();
    myMP3.play(track);
    return "done";
}

String volume(String data) {
	data.remove(0, 6);
	data.trim();
	int vol = data.toInt();
	myMP3.volume(vol);
	return "done";
}

String stop(String data) {
	myMP3.stop();
	Dbg_Println("stop");
	return "done";
}

String pause(String data) {
	myMP3.next();
	Dbg_Println("pause");
	return "done";
}

String resume(String data) {
	myMP3.start();
	Dbg_Println("start");
	return "done";
}

String next_play(String data) {
	myMP3.next();
	Dbg_Println("playnext");
	return "done";
}

String previous_play(String data) {
	myMP3.previous();
	Dbg_Println("playprevious");
	return "done";
}

String loop_track(String data) {
	data.remove(0, 4);
	data.trim();
	int track = data.toInt();
	myMP3.loop(track);
	return "done";
}

const UartCmdProcess CmdTable[NUMBER_OF_COMMAND_PROCESS_UART] =
{
	{"PLAYPREVIOUS", &previous_play},
	{"PLAYNEXT", &next_play},
    {"PLAY", &play}, {"VOLUME", &volume},{"STOP", &stop},
	{"PAUSE", &pause},{"RESUME", &resume}, {"LOOP", &loop_track},
};

bool UartHandle_Main()
{
	static unsigned long time = millis();
	String data = "";
	String cmd = "";
	String dataSend = "";
	String ret = "";
	
	// Lấy data nhận được từ Websocket buffer
	data = SerialGetCommand();
	if (data == "") return 0;
	data.trim();
	cmd = getIndex(data, ";");

	Dbg_Printf("Uart Cmd: ");
	Dbg_Println(cmd);
	data = cmd;
	// Kiểm tra trong bảng lệnh. Nếu khớp thì thực hiện lệnh xử lý tương ứng
	for (uint8_t i = 0; i < NUMBER_OF_COMMAND_PROCESS_UART; i++)
	{
		if (data.startsWith(CmdTable[i].cmd))
		{
			ret = CmdTable[i].process(data);
			dataSend = "\r\nCommand: " + cmd + ", process: " + ret;
			//WSClient_sendMsg(dataSend.c_str());
			return 1;
		}
	}
	return 0;
}