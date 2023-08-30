#ifndef __WIFI_EXTENDER_H_
#define __WIFI_EXTENDER_H_

#include <ESP8266WiFi.h>
#include <lwip/napt.h>
#include <lwip/dns.h>
#include <LwipDhcpServer.h>

#define NAPT		IP_NAPT_MAX
#define NAPT_PORT	IP_PORTMAP_MAX

typedef enum
{
	WFE_AP_ENABLE,
	WFE_AP_DISABLE,
	WFE_AP_NOINIT,
} WFE_APState_t;
enum 
{
	NAPT_DISABLE = 0U,
	NAPT_ENABLE = 1U,
};

class WiFiExtender
{
private:
	bool napt_init();
protected:

public:
	WiFiExtender() {};
	~WiFiExtender() {};
	String ap_ssid = "SMARTLIGHT_";
	String ap_psk = "1234567890";
	void set(const char* ssid, const char* psk);
	bool init();
	void deinit();
	void enable();
	void disable();
	WFE_APState_t APState = WFE_AP_NOINIT;
	bool naptInitDone = false;
};

#endif	/* __WIFI_EXTENDER_H_ */