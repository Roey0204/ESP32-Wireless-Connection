/*
 *Task Detect Development.
 *Acquire data in 20hz.
 *sent via ESP WiFi to another host ESP.
 @Author: Ong Roey Yee
*/

#include <Arduino.h>
#include <iostream>
#include <FastLED.h>

//!Include to talk to all 3 serial ports 
#include <HardwareSerial.h>

//Header from rig_control, rig setting
#include <rig_control.h>

//!ESPnow
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ntptime.h>
#include <ESP32Time.h>

NtpTime set;
ESP32Time timer ;

//ESP_now
//MAC Address of the PC dongle.
uint8_t broadcastAddress[] = {0xDC, 0xDA, 0x0C, 0x48, 0x94, 0x78};

//MAC Address of the Rig.
uint8_t newMACAddress[] =  {0xDC, 0xDA, 0x0C, 0x48, 0x96, 0x88};

CRGB leds[NUM_LEDS];
 
unsigned long lastSampleTime   = 0;     //Variable to calculate next sample time

// Init Flag
bool newData_Flag              = false; // Flag to indicate whether we received new ESP-NOW data
bool eventInProgress_Flag      = false; // Flag to stop 40Hz data sending while receiving ESP-NOW data


//!Function to communicate with DDM Board (requests data and sends target Power)
void updateData();

using namespace std;


//Storage for incoming/outgoing DIPC comms
struct rigControl_data_t {

  uint8_t param1;
  uint8_t param2;

} rigControl_data;

// Structure example to send data
typedef struct struct_message_feather_2_logger {
  uint32_t hour;
  uint32_t min;
  uint32_t sec;
  uint32_t millis;
  int32_t systemTime_ms;

} struct_message_feather_2_logger;

// Create a struct_message called sendData
struct_message_feather_2_logger sendData;

// Structure example to receive data
// Must match the receiver structure
typedef struct struct_message_logger_2_feather {
  uint8_t param1;
  uint8_t param2;

} struct_message_logger_2_feather;

// Create a struct_message_logger_2_feather called receivedData
struct_message_logger_2_feather receivedData;

esp_now_peer_info_t peerInfo;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");

  if (status != ESP_NOW_SEND_SUCCESS) {
    Serial.println("Last packet: Delivery Fail");
  }

}

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  eventInProgress_Flag = true; //block the loop from sending 40Hz requests
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  Serial.println("");
  Serial.println("Data received");

  rigControl_data.param1 = receivedData.param1;
  rigControl_data.param2= receivedData.param2;

  String received = "Parameter 1:"+String(rigControl_data.param1)+ ","+ "Parameter 2"+String(rigControl_data.param2);

  Serial.println(received);
  
  newData_Flag          = true;
  eventInProgress_Flag  = false;

}

void setup() {

  Serial.begin(115200); 
  delay(10);

  set.TimeStamp_begin();
  delay(10);

  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  if(NtpTime::connectionFlag){      
      leds[0] = CRGB::OrangeRed;
      FastLED.show();
      delay(1500);
      }

// Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
// Once ESPNow is successfully Init, we will register for recv CB to
// get recv packer info
  esp_now_register_recv_cb(OnDataRecv);

// Once ESPNow is successfully Init, we will register for Send CB to
// get the status of Transmitted packet
  esp_now_register_send_cb(OnDataSent);
  
// Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
// Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  Serial.println("Hello...");
  Serial.println("DIPC started");

//!Display MAC ADRESS
  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress());
  Serial.println("Setting initial modes");
  delay(100);



//!Complete setup
  Serial.println("Finished Setup");

}

void loop() {
  
//Wait until sample delay is complete (sample timer)
if(lastSampleTime + SAMPLE_DELAY <= millis()){


//Record sample time (used for sample timer)
  lastSampleTime = millis();

  // Send update request to DDM
  if (eventInProgress_Flag == false){

      updateData();
    }

  }

}

void updateData(){

  const int buff_len = 100; // Buffer Length
  byte in_buff[buff_len];

  //Ntp
  sendData.hour   = timer.getHour(true);
  sendData.min    = timer.getMinute();
  sendData.sec    = timer.getSecond();
  sendData.millis = timer.getMillis();

  Serial.println(sendData.hour);
  //Rig timer
  sendData.systemTime_ms = millis();

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));


  if (result == ESP_OK) {
      Serial.println("Sent data to PC with success");
  }
  else {
      Serial.println("Error sending the data to PC -- update data");
  }
}


