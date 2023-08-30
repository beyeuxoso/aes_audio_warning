
#include <cstring>
#include "AppWiFi.h"
#include "Dbg.h"
#include "SDCard.h"
#include "Memory.h"
#include "WiFiExtender.h"
#include "ESP8266Ping.h"
#include "WSClient.h"

static bool isConnectWiFi = false;
static bool isConnectInternet = false;
ESP8266WiFiMulti wifiMulti;
#if USING_WIFI_EXTENDER
WiFiExtender    WFExtender;
#endif

static bool helpcheckInternet()
{
    // Ping google to known ESP8266 is connect internet or not?
    bool ret = Ping.ping("www.google.com");
    return ret;
}

static uint8_t helpParseWiFiData(WiFiInf* WiFiInfPtr, String data_s)
{
    char* data_c = &data_s[0];
    char* data_ptr;
    uint8_t num = 0;
    String data_line;    
    
    do {
        if (num == 0) data_ptr = strtok(data_c, ";");
        else {
            //Chỉ dịnh đối số NULL trong hàm strtok để tiếp tục tách chuỗi ban đầu
            data_ptr = strtok(NULL, ";");
        }
        if (data_ptr != NULL)
        {
            data_line = String(data_ptr);
            int pos1 = data_line.indexOf("=");
            int pos2 = data_line.indexOf("/");
            if ((pos1 >= 0) && (pos1 < pos2))
            {
                WiFiInfPtr[num].ssid = data_line.substring(pos1 + 1, pos2);
                WiFiInfPtr[num].password = data_line.substring(pos2 + 1, data_line.length());
                num++;
            }
        }
    } while ((data_ptr != NULL) || (num >= WIFI_MULTI_NUMMBER_MAX));
    return num;
}

static uint8_t AppWiFi_GetCfg(WiFiInf* WiFiInfPtr)
{
    String data = "";
    // Get WiFi information from SDCard
    data = SDCard_LoadCfg(SD_WIFI);
    // Parse ssid & password, then save to WiFiInfPtr struct
    data.trim();
    if (data != "")
    {
        return(helpParseWiFiData(WiFiInfPtr, data));
    }
    return 0;
}

static void AppWiFi_addAP(String ssid, String pass)
{
    if (!wifiMulti.existsAP(ssid.c_str(), pass.c_str()))
    {
        // Add Wi-Fi networks you want to connect to
        wifiMulti.addAP(ssid.c_str(), pass.c_str());
    }
}

void AppWiFi_GetSSID()
{
    String ssid;
#if USING_WIFI_EXTENDER
    // Get info wifi (ssid & password) from scan wifi
    int num = WiFi.scanNetworks();
    for (uint8_t i = 0; i < num; i++)
    {
        if (WiFi.SSID(i).startsWith(WFExtender.ap_ssid))
        {
            AppWiFi_addAP(WiFi.SSID(i), WFExtender.ap_psk);
            Dbg_Println(WiFi.SSID(i));
        }
    }
#endif
    // Get info wifi (ssid & password) from allocate
    for (uint8_t i = 0; i < MAX_VARIABLE; i++)
    {
        if (memo.ListAllocateAddress[i].Name == "") continue;
        if (memo.ListAllocateAddress[i].Name != "" && memo.ListAllocateAddress[i].Type == MemoryType::WIFI) {

            String info = *memo.GetValue(memo.ListAllocateAddress[i].Name);
            String ssid = info.subString(0, info.indexOf("/"));
            String pass = info.substring(info.indexOf("/") + 1);
            AppWiFi_addAP(ssid,pass);
            Dbg_Print(ssid); Dbg_Println(pass);
        }
    }
}

void AppWiFi_Init()
{
    AppWiFi_GetSSID();
    Dbg_Printf("Connecting to WiFi\r\n");
    wifiMulti.run();

    int tries = 0;
    while((WiFi.status() != WL_CONNECTED) && (tries++ < WIFI_MULTI_RETRY_CONNECT))
    {
        delay(10);
        Dbg_Printf(".");
        wifiMulti.run();
    }
    if (tries < WIFI_MULTI_RETRY_CONNECT)
    {
        Dbg_Print("WiFi connected: ");
        Dbg_Print(WiFi.SSID());
        Dbg_Println();
        isConnectWiFi = true;
        // Khởi tạo tính năng phát wifi cho thiết bị khác
#if USING_WIFI_EXTENDER
        WFExtender.init();
        WFExtender.enable();
#endif
    }
    else
    {
        isConnectWiFi = false;
#if USING_WIFI_EXTENDER
        WFExtender.disable();
#endif
        Dbg_Printf("Connect WiFi failed\r\n");
    }
}

void AppWiFi_RefreshSSID()
{
    // Clean WiFi AP List
    wifiMulti.cleanAPlist();
}

void AppWiFi_ScanNetwork(dataType_t type, String* data)
{
    Dbg_Printf("Start scan Jnetwork");
    int num = WiFi.scanNetworks();
    String encrypt = "";
    String wifiInf = "";
    *data = "";
    if (num != 0)
    {
        if (type == JSON_DATA)
        {
            for (int i = 0; i < num; ++i)
            {
                encrypt = (WiFi.encryptionType(i) == ENC_TYPE_NONE ? "OPEN" : "SECURITY");
                String end = (i == num - 1 ? "}" : "},");
                wifiInf = "{\"SSID\":\"" + String(WiFi.SSID(i)) + "\", \"RSSI\":\"" + String(WiFi.RSSI(i)) + "\",\"ENCRYPTION\":\"" + encrypt + "\"" + end;
                wifiInf.trim();
                *data += wifiInf;
            }
        }
        else 
            if (type == NORMAL_DATA)
            {
                for (int i = 0; i < num; ++i)
                {
                    encrypt = (WiFi.encryptionType(i) == ENC_TYPE_NONE ? "OPEN" : "SECURITY");
                    wifiInf = "SSID:" + String(WiFi.SSID(i)).padleftString(50, " ") + ", RSSI:" + String(WiFi.RSSI(i)).padleftString(5, " ") + \
                              ", Encryption:" + encrypt.padleftString(11, " ") + "\r\n";
                    *data += wifiInf;
                }
            }
    }
}

void AppWiFi_Main()
{
    // Maintain WiFi connection
    if (wifiMulti.run(WIFI_MULTI_CONNECT_TIMEOUT) == WL_CONNECTED) {
            isConnectWiFi = true;
    }
    else {
        Dbg_Printf("Connect WiFi failed\r\n");
        isConnectWiFi = false;
    }
    if (isConnectWiFi == true) {       
#if USING_WIFI_EXTENDER                              // Có kết nôi Wifi -> Kiểm tra kết nối internet
        if (WSClient_CheckConnect() == false) {
            if (helpcheckInternet() == true) {       // Mất kết nối tới websoket server nhưng vẫn có kết nối internet (trường hợp websoket server lỗi)
                WFExtender.enable();
            }
            else {                                   // Mất kết nối internet
                WFExtender.disable();
            }
        }
        else {
            WFExtender.enable();
        }
#endif
    }
    else {                                          // Mất kết nối Wifi
        // Refresh wifi for multi wifi scan     
        AppWiFi_GetSSID();
#if USING_WIFI_EXTENDER
        WFExtender.disable();
#endif
    }
}