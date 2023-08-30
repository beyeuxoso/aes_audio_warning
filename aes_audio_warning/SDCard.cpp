
#include <SPI.h>
#include <SD.h>

#include "SDCard.h"
#include "Config.h"
#include "HelpString.h"
#include "Dbg.h"

void SDCard_Init()
{
	uint8_t retry = 0xFF;
	while ((!SD.begin(SD_CSPIN)) && (--retry))
	{
		delay(5);
	}
	if (retry > 0U)
	{
		Dbg_Printf("Connected SD Card\r\n");
	}
	else
	{
		Dbg_Printf("SD Card connect failed\r\n");
	}
}

String SDCard_ReadFile(String fname)
{
	File root;
	String data = "";
	root = SD.open(fname);
	if (root) {
		long ind = 0;
		while (root.available())
		{
			String kq = root.readString();
			data += kq;
			ind++;
		}
	}
	else
	{
		Dbg_Printf("Can not open file: ");
		Dbg_Println(fname);
	}
	root.close();
	return data;
}

static void SDCard_ReadFileJson(String fname)
{

}

void SDCard_WriteFile(String fname, String content, String endline)
{
	byte RecountFile = 5;
	uint8 indexStart = 0;
	uint8 indexEnd = 0;
	
	File root = SD.open(fname, FILE_WRITE);
	Dbg_Println("Write file:" + fname);
	if (root) {
		root.seek(EOF, SeekMode::SeekEnd);
		if (endline != "")
		{
			content.replace(endline, "\r\n");
			root.print(content);
		}
		else
			root.print(content);
		//SD.dateTimeCallback(dtcallback);			// comment by toanpv1
		root.close();
	}
	if (!SD.exists(fname) && RecountFile > 0) {
		SDCard_WriteFile(fname, content, endline);
		RecountFile--;
		if (RecountFile == 0)
		{
			RecountFile = 5;
			return;
		}
	}
}

static void SDCard_WriteFileJson(String fname)
{

}

static void SDCard_AppendFile(String fname)
{

}
bool SDCard_CreateDirectory(String dir)
{
	return SD.mkdir(dir);
}
bool SDCard_RemoveDirectory(String dir)
{
	return SD.rmdir(dir);
}

String SDCard_LoadCfg(CfgType type)
{
	String data = "";
	String path = "";
	switch (type)
	{
		case SD_WIFI:
			path = RootDir + WiFiDir;
			data = SDCard_ReadFile(path);
			break;
		case SD_SCREEN:
			path = RootDir + ScreenDir;
			data = SDCard_ReadFile(path);
			break;
		case SD_DEVICE_NAME:
			path = RootDir + DevNameDir;
			data = SDCard_ReadFile(path);
			break;
		case SD_Initialization:
			break;
			
		default:
			break;
	}
	return data;
}

void SDCard_SaveCfg()
{

}

uint16_t SDCard_ListFile(String dir, char* list, String exFile)
{
	File root;
	uint16_t num = 0;
	uint16_t idx = 0;
	bool isFullList = false;
	root = SD.open(dir);
	if (root)
	{
		root.rewindDirectory();
		while (true)
		{
			File entry = root.openNextFile();
			if (!entry)
			{
				break;
			}
			String name = entry.name();
			name.trim();
			if (name.endsWith(exFile))
			{
				for (uint8_t i = 0; i < name.length(); i++)
				{
					if (idx < 1024) {
						list[idx] = name[i];
						idx++;
					}
					else {
						isFullList = true;
					}
				}
				if (isFullList == false) {
					list[idx] = ';';
					idx++;
				}
				else {
					Dbg_Println("List file buffer is full.");
				}
				num++;
			}
			entry.close();
		}
		root.close();
		return num;
	}
	else
	{
		Dbg_Printf("Can not list file in dir: ");
		Dbg_Println(dir);
		return 0;
	}
}

/*
* 
* Param: 
*		- dir:		thư mục muốn liệt kê
*		- numTabs:	thứ tự
*/
String SDCard_ListDir(File& dir, int numTabs)
{
	//while (true) {

	//	File entry = dir.openNextFile();
	//	if (!entry) {
	//		// no more files
	//		break;
	//	}
	//	for (uint8_t i = 0; i < numTabs; i++) {
	//		Serial.print('\t');
	//	}
	//	Serial.print(entry.name());
	//	if (entry.isDirectory()) {
	//		Dbg_Println("/");
	//		SDCard_ListDir(entry, numTabs + 1);
	//	}
	//	else {
	//		// files have sizes, directories do not
	//		Serial.print("\t\t");
	//		Dbg_Println(entry.size(), DEC);
	//	}
	//	entry.close();
	//}
	return "";
}

String SDCard_LoadFile(String fname)
{
	return SDCard_ReadFile(fname);
}

bool SDCard_ReadFileStream(StreamInf_t* fstream, String fname)
{
	File file;
	bool ret;

	file = SD.open(fname);
	if (file)
	{
		fstream->cnt = 0U;
		file.seek(fstream->pos);
		while (file.available())
		{
			fstream->dataptr[fstream->cnt] = file.read();
			fstream->cnt++;
			if (fstream->cnt == fstream->size) break;
		}
	}
	else 
	{
		Dbg_Printf("Can not open file: ");
		Dbg_Println(fname);
		ret = false;
	}
	fstream->pos += fstream->cnt;
	ret = file.available();
	file.close();
	return ret;
}

bool SDCard_helpParseBatchFile(String dir, String* result, uint16_t timeout)
{
	String data = "";
	String command = "";
	int pos = 0;
	String batchFilenme = "";
	uint32_t timetick = millis();
	// Read file
	data = SDCard_ReadFile(dir);
	// Xóa bỏ phần tử xuống dòng
	data.replace("\n", "");
	data.replace("\r", "");
	// Tìm vị trí của dấu ';'
	while ((pos = data.indexOf(';')) != -1) {
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
			SDCard_helpParseBatchFile(batchFilenme, result);
		}
		else
		{
			if (!command.startsWith("//"))
			{
				*result += command;
			}
		}
		if ((millis() - timetick) > timeout) return false;
	}
	return true;
}