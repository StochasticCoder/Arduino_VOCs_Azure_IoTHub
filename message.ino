#include "Adafruit_CCS811.h"
#include <ArduinoJson.h>
#include <SimpleDHT.h>

Adafruit_CCS811 ccs;
SimpleDHT11 dht(DHT_PIN);

void initSensor()
{    
  if(!ccs.begin())
  {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }

  //calibrate temperature sensor
  while(!ccs.available());  

  int i =0;

  // Loop 4 times to reset ccs 
  while(i < 5)
  {
    if(ccs.available())
    {
      float temp = ccs.calculateTemperature();
      if(!ccs.readData())
      {
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print("ppb   Temp:");
        Serial.println(temp);
      }
      else{
        Serial.println("ERROR!");
        while(1);
      }
  }

  delay(1000);
  i=i+1;
  }
}



void readMessage( char *payload)
{
  byte temperature = 0;
  byte humidity = 0;
  float co2 = 0;
  float tvoc = 0;
  
  if(!ccs.readData()){
    co2 = ccs.geteCO2();
    tvoc = ccs.getTVOC();
  }
  
  dht.read(&temperature,&humidity,NULL);
  
  temperature = temperature*9/5+32;
  DynamicJsonDocument root(1024);
  
  root["device"] = DEVICE_ID;
  root["location"] = location;
  
  
  //NAN is not the valid json, change it to NULL
  
  if (std::isnan(temperature))
  {
    root["temperature"] = NULL;
  }
  else
  {
    root["temperature"] = temperature;
  }
  
  if (std::isnan(humidity))
  {
    root["humidity"] = NULL;
  }
  else
  {
    root["humidity"] = humidity;
  }
  
  if (std::isnan(co2))
  {
    root["CO2"] = NULL;
  }
  else
  {
    root["CO2"] =  co2;
  }
  
  if (std::isnan(tvoc))
  {
    root["TVOC"] = NULL;
  }
  else
  {
    root["TVOC"] = tvoc;
  }
  
  
  serializeJson(root,payload,MESSAGE_MAX_LEN);

}



void parseTwinMessage(char *message)
{
    Serial.println("Parse Twin");
    DynamicJsonDocument root(1024);
    
    deserializeJson(root, message,DeserializationOption::NestingLimit(15));
   
    if (root.isNull())
    {
        Serial.printf("Parse %s failed.\r\n", message);
        return;
    }

    if (!root["desired"]["interval"].isNull())
    {
        interval = root["desired"]["interval"];
    }
    else if(root.containsKey("interval"))
    {
       interval = root["interval"];
    }


     if (!root["desired"]["location"].isNull())
    {
        location = root["desired"]["location"].as<String>();
    }
    else if(root.containsKey("location"))
    {
       location = root["location"].as<String>();
    }
}
