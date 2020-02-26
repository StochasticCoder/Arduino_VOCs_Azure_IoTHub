#include <ESP8266WiFi.h>
#include <Wire.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

#include "config.h"

int status = WL_IDLE_STATUS;

/************ Change Connection String*************/
char* conStr  = "<Add you device SAS Tocken from the Device Explorer>";
/*****************************************************************/

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
int interval = INTERVAL;
String location = "unknown";

static bool messagePending = false;
static bool messageSending = true;


void blinkLED()
{
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

void initSerial()
{
  Serial.begin(9600);
  Serial.println("CCS811 test");  
}

void initWifi()
{
  Serial.println("Init WiFi");  
  if(WiFi.status() == WL_NO_SHIELD)
  {
    Serial.println("Wifi shield not present");
    while(true);
  }
  
  //***** Change SSID and Pass **********************
  status = WiFi.begin("<WIFI SSID","WIFI PASS");
  //*****************************************************
  
  delay(3000);
  
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("wait...");
  }
}



void setup() {
  delay(100);
  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(LED_PIN, OUTPUT); 
  
  initSerial();
  initWifi();
  initSensor();

  Serial.println("get iot hub client...");
  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(conStr, MQTT_Protocol);
  
  if(iotHubClientHandle == NULL)
  {
    Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
    while (1);
  }

  Serial.println("got client...");
  IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "VOC-Monitor");
  if(IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL)!= IOTHUB_CLIENT_OK)
  {
    Serial.println("ERROR: IoTHubClient_LL_SetMessageCallback..........FAILED!");
  }
 
  IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
  IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);

  Serial.println("do work...");
  IoTHubClient_LL_DoWork(iotHubClientHandle);
}


void loop() {
 
  if (!messagePending && messageSending)
  {  
     Serial.println("Prep Message....");
    char messagePayload[MESSAGE_MAX_LEN];
    readMessage( messagePayload);    
    sendMessage(iotHubClientHandle, messagePayload);
    Serial.println(messagePayload); 
    delay(interval);
  }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    
  delay(10);
}
