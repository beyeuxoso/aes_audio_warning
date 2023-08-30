
#include "Fota.h"
#include "FTP_Client.h"
#include <Updater.h>
#include "SDCard.h"
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

FTP_Client ftp;
HTTPClient http;
WiFiClient client;
static void progressCallBack(size_t curSize, size_t totalSize) 
{
    FotaDbg_Printf("Update process at %d of %d bytes...\n", curSize, totalSize);
}
static uint32_t FTP_checkFile(String fName)
{
    uint32_t fSize = 0;
    String list[128];
    //Get the directory content in order to allocate buffer
    //my server response is type=file;size=18;modify=20190731140703;unix.mode=0644;unix.uid=10183013;unix.gid=10183013;unique=809g7c8e92e4; helloworld.txt
    
    ftp.contentList("", list);
    for (uint8_t i = 0; i < sizeof(list); i++)
    {
        uint8_t indexSize = 0;
        uint8_t indexMod = 0;

        if (list[i].length() > 0)
        {
            list[i].toLowerCase();

            if (list[i].indexOf(fName) > -1)
            {
                indexSize = list[i].indexOf("size") + 5;
                indexMod = list[i].indexOf("modify") - 1;
                fSize = list[i].substring(indexSize, indexMod).toInt();
            }

            //Print the directory details
            //FotaDbg_Println(list[i]);
        }
        else
            break;
    }
    return fSize;
}

void Fota_Download(String type, String url, String user, String pass, String fName, uint16_t bufferSize, String* ret)
{
    bool result;
    if (type.startsWith("FTP"))
    {
        // Check firmware
        //Connect to FTP server
        ftp.begin(url.c_str(), user.c_str(), pass.c_str(), 15000);
        result = ftp.openConnection();
        if (result == false)
        {
            *ret = "[Check file]Can not connect to FTP server";
            return;
        }
        //Choose the work directory
        ftp.changeWorkDir("/Firmware");
        ftp.initFile(FTP_FileType_BINARY);
        uint32_t fsize = FTP_checkFile(fName);
        FotaDbg_Printf("File size: ");
        FotaDbg_Println(fsize);
        ftp.closeConnection();
        ftp.end();
        if ((bufferSize < 128) || (bufferSize >=1024)) {
            bufferSize = 512;
        }
        if (fsize > 0)
        {
            // Download firmware
            ftp.begin(url.c_str(), user.c_str(), pass.c_str(), 15000);
            result = ftp.openConnection();
            if (result == false)
            {
                *ret = "[Download]Can not connect to FTP server";
                return;
            }
            //Choose the work directory
            ftp.changeWorkDir("/Firmware");
            //Download file then save to SD card
            ftp.initFile(FTP_FileType_BINARY);
            result = ftp.downloadFirmware(fName.c_str(), fsize, bufferSize);
            if (result == true)
            {
                *ret = "Download firmware completed";
            }
            else
            {
                *ret = "Download firmware failed";
            }
        }
        else
        {
            *ret = "Firmware is empty";
        }
        ftp.closeConnection();
        ftp.end();
    }
    if (type.startsWith("HTTP"))
    {
        //http://private-iot.homeos.vn/firmwares/may21/Updating/Wifi_511.bin
        String link = url + "/" + fName;
        http.begin(client, link);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) 
        {
            FotaDbg_Println("Starting download...");
            String filename = RootDir + FirmwareDir + fName;
            if (SD.exists(filename))
            {
                SD.remove(filename);
            }
            File file = SD.open(filename, FILE_WRITE);
            if (file) 
            {
                Stream* response = http.getStreamPtr();
                uint8_t buffer[1024];
                size_t length;
                size_t written;
                while ((length = response->readBytes(buffer, sizeof(buffer))) > 0) 
                {
                    written = file.write(buffer, length);
                    if (written != length) {
                        FotaDbg_Println("Failed to write to SD card");
                        *ret = "Failed to write to SD card";
                        break;
                    }
                }
                file.close();
                FotaDbg_Println("Download completed");
                *ret = "Download completed";
            }
            else {
                FotaDbg_Println("Failed to open file for writing");
                *ret = "Failed to open file for writing";
            }
        }
        else 
        {
            FotaDbg_Printf("HTTP error: ");
            FotaDbg_Println(httpCode);
            *ret = "HTTP error: " + String(httpCode);
        }
    }
}

void Fota_Update(String filename)
{
    String fName = RootDir + FirmwareDir + filename;
    File file = SD.open(fName);
    if (file) {
        FotaDbg_Println("Start update");
        Update.onProgress(progressCallBack);
        Update.begin(file.size(), U_FLASH);
        Update.writeStream(file);
        if (Update.end())
        {
            FotaDbg_Println(F("Update finished!"));
            delay(1000);
            ESP.reset();
        }
        else {
            FotaDbg_Println(F("Update error!"));
            FotaDbg_Println(Update.getError());
        }
    }
    file.close();
}