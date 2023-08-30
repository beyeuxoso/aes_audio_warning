
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "NTPClient.h"
#include "ntp.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void NTP_Init() {
    // Initialize a NTPClient to get time
    timeClient.begin();
    // Set offset time in seconds to adjust for your timezone
    timeClient.setTimeOffset(TIME_OFFSET);
}

void NTP_Gettime(tm* time) {
    if (WiFi.isConnected())
    {
        // Update time from NTP server
        timeClient.update();
        time_t epochTime = timeClient.getEpochTime();
        //Get a time structure
        struct tm* ptm = gmtime((time_t*)&epochTime);
        time->tm_year  = ptm->tm_year + 1900;
        time->tm_mon   = ptm->tm_mon + 1;
        time->tm_mday  = ptm->tm_mday;
        time->tm_hour  = timeClient.getHours();
        time->tm_min   = timeClient.getMinutes();
        time->tm_sec   = timeClient.getSeconds();
    }
}