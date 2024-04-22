#include <WiFi.h>
#include "time.h"
#include <ctime>
#include <ESP32Time.h>
#include "time.h"
#include <sys/time.h>
#include <ntptime.h>

struct timeval tv;
ESP32Time rtc;  
const char* ssid     = "xxx";
const char* password = "xxx!";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 28800;      //Using GMToffset instead of daylightOffset , Malaysia = +8 hours
const int   daylightOffset_sec = 0;
struct tm timeinfo;                     // global variable for use in rtc 

void NtpTime ::TimeStamp_begin(){

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); 

  // Checking the flag of obtaining time & pass the ntp time to timeinfo buffer
  if(!getLocalTime(&timeinfo)){

    Serial.println("Failed to obtain time");   
    return;
  }

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}


int NtpTime ::GetMonth(){return rtc.getMonth()+1;}
int NtpTime ::GetDay(){return rtc.getDay();}
int NtpTime ::GetYear(){return rtc.getYear();}
int NtpTime ::GetHour(){return rtc.getHour(true);}
int NtpTime ::GetMin(){return rtc.getMinute();}
int NtpTime ::GetSec(){return rtc.getSecond();}


