#include "FTP_Client.h"
#include "SDCard.h"
#include <Esp.h>

//######################################################################################################
void    FTP_Client::begin(const char* _serverAdress,const  char* _userName, const char* _passWord, uint16_t _timeout)
{
    serverAdress = (char*)_serverAdress;
    userName = (char*)_userName;
    passWord = (char*)_passWord;    
    timeout = _timeout;   
    FTP_Printf("[FTP_Client] Begin.\r\n");
}
//######################################################################################################
bool    FTP_Client::openConnection(void)
{
    char    ans[128];
    if (client.connect(serverAdress, 21))
    {
        client.setTimeout(timeout);
        FTP_Printf("[FTP_Client] Connected.\r\n");
    }
    else
    { 
        FTP_Printf("[FTP_Client] Can not Connect!\r\n");
        return false;
    } 
    getFTPAnswer(ans);
    FTP_Printf("[FTP_Client] Send: USER %s\r\n",userName);
    client.print("USER ");
    client.println(userName);
    getFTPAnswer(ans);
    FTP_Printf("[FTP_Client] Send: PASS %s\r\n",passWord);
    client.print("PASS ");
    client.println(passWord);
    getFTPAnswer(ans);
    if(strstr(ans,"230-OK") == NULL)
    {     
        FTP_Printf("[FTP_Client] Can not login! check your user,pass\r\n");
        return false;
    }   
    FTP_Printf("[FTP_Client] Send: SYST\r\n");
    client.println("SYST");
    getFTPAnswer(ans);
    return true;
}
//######################################################################################################
void FTP_Client::closeConnection(void)
{  
    FTP_Printf("[FTP_Client] Send: QUIT\r\n");
    client.println("QUIT");
    client.stop();
}
//######################################################################################################
bool FTP_Client::getFTPAnswer(char* result, int offsetStart)
{
    char thisByte;
    outCount = 0;
    unsigned long _m = millis();
    memset( outBuf, 0, sizeof(outBuf));
    if(result != NULL)
        result[0] = 0;
    while (!client.available() && millis() < _m + timeout)
        delay(1);
    delay(10);
    if(!client.available())
    {
        memset( outBuf, 0, sizeof(outBuf));
        strcpy( outBuf, "Offline");
        isConnected = false;
        FTP_Printf("[FTP_Client] Offline!\r\n");
        return false;
    }
    isConnected = true;
    while (client.available())
    {
        thisByte = client.read();
        if (outCount < sizeof(outBuf))
        {
            outBuf[outCount] = thisByte;
            outCount++;
            outBuf[outCount] = 0;
        }
    }
    if(result != NULL)
    {
        for(size_t i = offsetStart; i<sizeof(outBuf); i++)
            result[i] = outBuf[i - offsetStart];
        
        FTP_Print("[FTP_Client] Result:\r\n");
        FTP_Print(outBuf);
        
    }
    if(outBuf[0] == '4' || outBuf[0] == '5' )
    {
        FTP_Printf("[FTP_Client] Not Accept!\r\n");
        return false;
    }
    return true;
}
//######################################################################################################
bool FTP_Client::renameFile(const char* from,const char* to)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: RNFR %s\r\n",from);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("RNFR ");
    client.println(from);
    getFTPAnswer(ans);
    if(strstr(ans,"350") == NULL)
        return false;
    FTP_Printf("[FTP_Client] Send: RNTO %s\r\n",to);
    client.print("RNTO ");
    client.println(to);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::newFile (const char* fileName)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: STOR %s\r\n",fileName);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("STOR ");
    client.println(fileName);
    getFTPAnswer(ans);
    if(strstr(ans,"150") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::initFile(FTP_FileType_t FTP_FileType)
{
    char ans[128];
    
    if (FTP_FileType == FTP_FileType_ASCII) {
        FTP_Printf("[FTP_Client] Send: TYPE A\r\n");
    }   
    else {
        FTP_Printf("[FTP_Client] Send: TYPE I\r\n");
    }
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    if (FTP_FileType == FTP_FileType_ASCII)
        client.println("TYPE A");
    else
        client.println("TYPE I");    
    getFTPAnswer(ans);
    if(strstr(ans,"200") == NULL)
    {
        FTP_Printf("[FTP_Client] Not Accept TYPE\r\n");
        return false;
    }
    FTP_Printf("[FTP_Client] Send: PSAV\r\n");
    client.println("PASV");
    getFTPAnswer(ans);
    if((strstr(ans,"227") == NULL) && (strstr(ans,"228") == NULL) && (strstr(ans,"229") == NULL))
    {
        FTP_Printf("[FTP_Client] Not Accept PSAV\r\n");
        return false;
    }
    char *tStr = strtok(outBuf, "(,");
    int array_pasv[6];
    for ( int i = 0; i < 6; i++)
    {
        tStr = strtok(NULL, "(,");
        if (tStr == NULL)
        {
            FTP_Println(F("[FTP_Client] Bad PASV Answer"));
            return false;
        }
        array_pasv[i] = atoi(tStr);
    }
    unsigned int hiPort, loPort;
    hiPort = array_pasv[4] << 8;
    loPort = array_pasv[5] & 255;
    hiPort = hiPort | loPort;
    FTP_Print(F("[FTP_Client] Data port: "));
    FTP_Println(hiPort);
    if (dclient.connect(serverAdress, hiPort))
    {
        dclient.setTimeout(timeout);
        FTP_Println(F("[FTP_Client] Data connection established "));
        return true;
    }
    FTP_Println(F("[FTP_Client] Data connection Not established!"));
    return false;
}
//######################################################################################################
bool FTP_Client::appendFile (const char* fileName)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: APPE %s\r\n",fileName);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("APPE ");
    client.println(fileName);
    getFTPAnswer(ans);
    if(strstr(ans,"150") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::changeWorkDir(const char * dir)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: CWD %s\r\n",dir);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("CWD ");
    client.println(dir);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::deleteFile(const char * file)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: DELE %s\r\n",file);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("DELE ");
    client.println(file);
    getFTPAnswer(ans);
    if(strstr(ans,"250") == NULL)
        return false;
    else
        return true;
}
//######################################################################################################
bool FTP_Client::makeDir(const char * dir)
{
    char ans[128];
    FTP_Printf("[FTP_Client] Send: MKD %s\r\n",dir);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("MKD ");
    client.println(dir);
    getFTPAnswer(ans);
    if(strstr(ans,"257") == NULL)
        return false;
    else
        return true;    
}
//######################################################################################################
bool FTP_Client::writeData (uint8_t * data, int dataLength)
{
    FTP_Printf("[FTP_Client] Writing %d Bytes in File\r\n",dataLength);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    if(writeClientBuffered(&dclient, data, dataLength) == false)
    {
        FTP_Printf("[FTP_Client] Writing Error!\r\n");
        return false;
    }
    else
    {
        FTP_Printf("[FTP_Client] Writing Done.\r\n");
        return true;
    }
}
//######################################################################################################
bool FTP_Client::writeData (const uint8_t * data, int dataLength)
{
    FTP_Printf("[FTP_Client] Writing %d Bytes in File\r\n",dataLength);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    if(writeClientBuffered(&dclient, data, dataLength) == false)
    {
        FTP_Printf("[FTP_Client] Writing Error!\r\n");
        return false;
    }
    else
    {
        FTP_Printf("[FTP_Client] Writing Done.\r\n");
        return true;
    }
}
//######################################################################################################
bool FTP_Client::closeFile (void)
{
    FTP_Printf("[FTP_Client] Close File\r\n");
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    dclient.stopAll();
    return true;
}
//######################################################################################################
bool FTP_Client::write(const char * str)
{
    FTP_Printf("[FTP_Client] Writing string in File\r\n");
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    int printedByte = dclient.print(str);
    if(printedByte == (int)strlen(str))
    {
        FTP_Printf("[FTP_Client] Writing %d Bytes in File, Done\r\n",printedByte);
        return true;
    }
    else
    {
        FTP_Printf("[FTP_Client] Writing only %d Bytes in File, Faild!\r\n",printedByte);
        return false;
    }
}
//######################################################################################################
bool FTP_Client::getLastModifiedTime(const char  * fileName, char* result)
{
    FTP_Printf("[FTP_Client] Send: MKD %s\r\n",fileName);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("MDTM ");
    client.println(fileName);
    getFTPAnswer(result, 4);
    if(result[0] == '2')
        return true;
    else
        return false;
}
//######################################################################################################
bool FTP_Client::writeClientBuffered(WiFiClient* cli, const uint8_t *data, int dataLength)
{
    size_t clientCount = 0;
    size_t cnt;
    for(int i = 0; i < dataLength;i++)
    {
        clientBuf[clientCount] = pgm_read_byte(&data[i]);
        clientCount++;
        if (clientCount > sizeof(clientBuf)-1)
        {
            cnt = cli->write(clientBuf, sizeof(clientBuf)); 
            if(cnt != sizeof(clientBuf))
                return false;
            clientCount = 0;
        }
    }
    if (clientCount > 0)
        cnt = cli->write(clientBuf, clientCount); 
    if(cnt != clientCount)
        return false;
    else
        return true;    
}
//######################################################################################################
bool FTP_Client::writeClientBuffered(WiFiClient* cli,uint8_t *data, int dataLength)
{
    size_t clientCount = 0;
    size_t cnt;
    for(int i = 0; i < dataLength;i++)
    {
        clientBuf[clientCount] = data[i];
        clientCount++;
        if (clientCount > sizeof(clientBuf)-1)
        {
            cnt = cli->write(clientBuf, sizeof(clientBuf)); 
            if(cnt != sizeof(clientBuf))
                return false;
            clientCount = 0;
        }
    }
    if (clientCount > 0)
        cnt = cli->write(clientBuf, clientCount); 
    if(cnt != clientCount)
        return false;
    else
        return true;   
}
//######################################################################################################
bool FTP_Client::contentList(const char * dir, String * list) 
{
    char _resp[ sizeof(outBuf) ];
    uint16_t _b = 0;  
    FTP_Printf("[FTP_Client] Send: MLSD %s\r\n",dir);
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("MLSD ");
    client.println(dir);
    getFTPAnswer(_resp);
    unsigned long _m = millis();
    while( !dclient.available() && millis() < _m + timeout) delay(1);
    while(dclient.available()) 
    {
        if( _b < 128 )
        {
            list[_b] = dclient.readStringUntil('\n');
            _b++;
        }
    }
    return true;
}
//######################################################################################################
bool FTP_Client::downloadString(const char * filename, String &str)
{
    FTP_Println("Send RETR");
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("RETR ");
    client.println(filename);
    char _resp[ sizeof(outBuf) ];
    getFTPAnswer(_resp);
    if(strstr(_resp,"150") == NULL)
    {
        FTP_Printf("[FTP_Client] Can not open File: %s \r\n",filename);
        return false;
    }
    unsigned long _m = millis();
    delay(100);
    while(1)
    {
        delay(1);
        if(dclient.available())
            str += dclient.readString();
        else
            break;
        if( millis() > _m + timeout )
            break;
    }    
    return true;
}
//######################################################################################################
bool FTP_Client::downloadFile(const char * filename, unsigned char * buf, size_t length)
{
    FTP_Println("Send RETR");
    if(!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("RETR ");
    client.println(filename);
    char _resp[ sizeof(outBuf)];    
    getFTPAnswer(_resp);
    unsigned long _m = millis();
    while( !dclient.available() && millis() < _m + timeout) delay(1);
    while(dclient.available()) 
        dclient.readBytes(buf, length);
    return true;
}

//######################################################################################################
bool FTP_Client::downloadFirmware(const char* filename, size_t filesize, uint16_t bufferSize,uint32_t timeout)
{
    char buffer[bufferSize];
    uint32_t bytes = 0;
    uint32_t fsizeRemain = filesize;
    uint32_t startTime = millis();
    FTP_Println("Send RETR");
    if (!isConnected)
    {
        FTP_Printf("[FTP_Client] Offline\r\n");
        return false;
    }
    client.print("RETR ");
    client.println(filename);
    char _resp[sizeof(outBuf)];
    getFTPAnswer(_resp);
    // Remove file if file exist
    String fName = RootDir + FirmwareDir + filename;
    if (SD.exists(fName)) {
        SD.remove(fName);
    }
    // Create file
    File file = SD.open(fName, FILE_WRITE);
    if (!file) {
        FTP_Println("File creation failed.");
        return 0;
    }
    while (dclient.available() || (fsizeRemain > 0))
    {
        if (dclient.available())
        {
            FTP_Printf("Download %d bytes, remain %d bytes\r\n", bufferSize, fsizeRemain);
            bytes = (fsizeRemain > bufferSize) ? bufferSize : fsizeRemain;
            fsizeRemain -= bytes;
            dclient.readBytes(buffer, bytes);
            // Write data to SD
            file.write(buffer, bytes);
            // Reset Watchdog
            ESP.wdtFeed();
        }
        else
        {
            FTP_Printf("Not available");
        }
        if ((millis() - startTime) > timeout)
        {
            FTP_Printf("Timeout.\r\n");
            file.close();
            if (SD.exists(fName)) {
                SD.remove(fName);
            }
            return false;
        }
    }
    file.close();
    FTP_Printf("Download completed: %d (ms)",(millis() - startTime));
    return true;
}
//######################################################################################################
void FTP_Client::end()
{
    serverAdress = NULL;
    userName = NULL;
    passWord = NULL;
    timeout = 10000;
    FTP_Printf("[FTP_Client] End.\r\n");
}