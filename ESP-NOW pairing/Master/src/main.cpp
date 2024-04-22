#include <Arduino.h>
// Libraries for WiFi communication
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ntptime.h>
#include <ESP32Time.h>
#include <FastLED.h>

#define DATA_PIN 48
#define NUM_LEDS 1

NtpTime set;
ESP32Time timer ;

CRGB leds[NUM_LEDS];
 
// Variables

unsigned long t = 0;       // Variable to store starting time.

String modified_millis = "";
//////////////////////////////// ESP-NOW VARIABLES /////////////////////////////////
// The MAC Address of the device sending the DISCO data
uint8_t broadcastAddress[] = {0xDC, 0xDA, 0x0C, 0x48, 0x96, 0x88};

// MAC address of the dongle
uint8_t newMACAddress[] = {0xDC, 0xDA, 0x0C, 0x48, 0x94, 0x78};

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message_feather_2_logger {
  uint32_t hour;
  uint32_t min;
  uint32_t sec;
  uint32_t millis;
  int32_t systemTime_ms;

} struct_message_feather_2_logger;

// Create a struct_message_feather_2_logger called receivedData
struct_message_feather_2_logger receivedData;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message_logger_2_feather {
  uint8_t param1;
  uint8_t param2;

} struct_message_logger_2_feather;

// Create a struct_message_logger_2_feather called sendData
struct_message_logger_2_feather sendData;

/////////////////////////////// MODE VARIABLES //////////////////////////////

// Switch to true to get Serial debug out
boolean debug = false;

//////////////////////////////// WIFI VARIABLES /////////////////////////////////

// ESP-NOW variable
esp_now_peer_info_t peerInfo;

///////////////////////////// LOOP FUNCTIONS ////////////////////////////////////////

// callback function that will be executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receivedData, incomingData, sizeof(receivedData));

  if(receivedData.millis<10){
    modified_millis = "00"+String(receivedData.millis);
  }
  else if ((receivedData.millis>=10)&&(receivedData.millis<100)){
    modified_millis = "0"+String(receivedData.millis);
  }
  else{
    modified_millis = String(receivedData.millis);
  }

  //timestamp from Rig ESP
  String timestamp = String(receivedData.hour) +":"+ String(receivedData.min)+":"+ String(receivedData.sec)+"."+ modified_millis;

  //Create string to send over Serial
  String serialMessage = String(timestamp) + "," +
                          String(receivedData.systemTime_ms) +"\r\n"; 

  Serial.print(serialMessage);

}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  if (debug) {
    Serial.print("\r\nLast Packet Send Status:\t");
    Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  }
}

//////////////////////////////////////////// SETUP ///////////////////////////////////////////////////

void setup() {
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  // Initialize Serial Monitor
  Serial.begin(115200);
  delay(10);  
  set.TimeStamp_begin();
  delay(10);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
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

    leds[0] = CRGB::Purple;
    FastLED.show();

}

/////////////////////////////////////////////// MAIN LOOP ////////////////////////////////////////////

// Parse csv values typed into terminal
void loop() {
 
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil(10);        //	10=LF (line feed - new line)
    int comma1  = input.indexOf(','); 
    int comma2  = input.indexOf(',', comma1+1);

    uint8_t param1 = input.substring(0,comma1).toInt();
    if (debug) {
      Serial.print("param1: ");
      Serial.println(param1);
    }
    sendData.param1 = param1;

     uint8_t param2 = input.substring(comma1+1,comma2).toInt();
    if (debug) {
      Serial.print("param2:");
      Serial.println(param2);
    }
    sendData.param1 = param2;

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));
  
  }
}