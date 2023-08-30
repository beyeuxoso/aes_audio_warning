
#include "WiFiExtender.h"
#include "Dbg.h"

#if LWIP_FEATURES && !LWIP_IPV6

bool WiFiExtender::napt_init()
{
    bool ret = true;
    //err_t err = ip_napt_init(NAPT, NAPT_PORT);
    err_t err = ip_napt_init(256U, 16U);
    if (err == ERR_OK) {
        err = ip_napt_enable_no(SOFTAP_IF, NAPT_ENABLE);
    }
    if (err != ERR_OK) {
        Dbg_Printf("NAPT initialization failed\n");
        naptInitDone = false;
        ret = false;
    }
    else
    {
        Dbg_Printf("NAPT initialization succeed\n");
        naptInitDone = true;
        ret = true;
    }
    return ret;
}

void WiFiExtender::set(const char* ssid, const char* psk)
{
    ap_ssid = String(ssid);
    ap_psk = String(psk);
}

bool WiFiExtender::init()
{
    bool ret = true;
    if (WiFi.status() == WL_CONNECTED)
    {
        WiFi.mode(WIFI_STA);
        // Give DNS servers to AP side
        dhcpSoftAP.dhcps_set_dns(0, WiFi.dnsIP(0));
        // Enable AP, with android-compatible google domain
        WiFi.softAPConfig(  
            IPAddress(172, 217, 28, 254),
            IPAddress(172, 217, 28, 254),
            IPAddress(255, 255, 255, 0));
        // Init Softwares Access Point
        WiFi.softAP(ap_ssid, ap_psk);
        ret = napt_init();
    }
    else
    {
        Dbg_Printf("WiFi is not connected. Please connect to WiFi\n");
        ret = false;
    }
    return ret;
}
// After execute deinit function, esp8266 must be restart to reinit napt.
void WiFiExtender::deinit()
{
    // Ngắt kết nối AP mềm
    WiFi.softAPdisconnect(true);
    // Tắt chế độ WiFi
    WiFi.mode(WIFI_OFF);
    Dbg_Println("NAT and WiFi mode ended.");
}

void WiFiExtender::enable()
{
    if (naptInitDone == false)
    {
        napt_init();
    }
    if (APState != WFE_AP_ENABLE)
    {
        Dbg_Println("WiFi extender enable AP");
        WiFi.softAP(ap_ssid + ESP.getChipId(), ap_psk);
        APState = WFE_AP_ENABLE;
    }
}

void WiFiExtender::disable()
{
    if (APState != WFE_AP_DISABLE)
    {
        Dbg_Println("WiFi extender disable AP");
        WiFi.softAP("_" + ap_ssid + ESP.getChipId(), ap_psk);
        APState = WFE_AP_DISABLE;
    }
}
#else
#error NAPT not supported in this configuration
#endif