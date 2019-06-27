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
  delay(1000);
  Wire.begin(5, 4);
  pinMode(D0, INPUT);
  pinMode(LED_PIN, OUTPUT); 
  
  initSerial();
  initWifi();
  initSensor();
  
  iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(conStr, MQTT_Protocol);
  
  if(iotHubClientHandle == NULL)
  {
    Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
    while (1);
  }
  
  IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "VOC-Monitor");
  IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
  IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
  IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, NULL);

  IoTHubClient_LL_DoWork(iotHubClientHandle);
}


void loop() {

  if (!messagePending && messageSending)
  {  
    char messagePayload[MESSAGE_MAX_LEN];
    readMessage( messagePayload);    
    sendMessage(iotHubClientHandle, messagePayload);
    Serial.println(messagePayload);
    delay(interval);
  }
    IoTHubClient_LL_DoWork(iotHubClientHandle);
    
  delay(10);
}
