#include <WiFi.h>
#include "time.h"
#include <ctime>
#include <ESP32Time.h>
#include "time.h"
#include <sys/time.h>
#include <ntptime.h>

bool NtpTime::connectionFlag = false;
struct timeval tv;
ESP32Time rtc;  

const char* ssid                           = "xxx";
const char* password                       = "xxx!";
const char* ntpServer                      = "pool.ntp.org";
const long  gmtOffset_sec                  = 28800;      //Using GMToffset instead of daylightOffset
const int   daylightOffset_sec             = 0; 
unsigned long CONNECTION_KICK_OUT_TIMER_MS = 45000;
unsigned long previousMillis               = 0;
struct tm timeinfo;

void NtpTime ::TimeStamp_begin(){
  Serial.print("Connecting to ");

  String ntpInfo =  "ID: "+ String(ssid)+"\n"+
                    "PW: "+ String(password)+"\n"+
                    "Server: "+ String(ntpServer)+"\n"+
                    "gmtOffset_sec: "+ String(gmtOffset_sec)+"\n"+
                    "daylightOffset_sec: "+ String(daylightOffset_sec)+"\n";
                  
  Serial.println(ntpInfo);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
      unsigned long currentMillis = millis();
      if(currentMillis-previousMillis>=CONNECTION_KICK_OUT_TIMER_MS){
        Serial.println("Failed to connect NTP SERVER");
        NtpTime::connectionFlag = true;
        break;
     }

    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  if (WiFi.status()==WL_CONNECTED){Serial.println("WiFi connected."); NtpTime::connectionFlag = false;}

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

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




