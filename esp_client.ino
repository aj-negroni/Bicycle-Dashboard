#include "HX711.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>

const int LOADCELL_DOUT_PIN = 0;
const int LOADCELL_SCK_PIN = 2;

HX711 scale;
float reading = 0.0;

ESP8266WiFiMulti WiFiMulti;
HTTPClient http, http2;
int httpRC = -125, http2RC = -125;

// power meter variables
float cadence = 0.0, power = 0.0, MAX = -1000000.0;
int indexO = 0, indexM = 0, count = 0, buffCount = 0;
float slope = 0.0, prev_edge = 0, ZeroValue = 0.0; 
float summation = 0.0, previous = 0.0, ForceSum = 0;
float power_sum = 0.0;
int power_count = 0, startTime = 0, endTime = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println();
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("ESPsoftAP_01", "pass-to-soft-AP");
  while((WiFiMulti.run() == WL_CONNECTED)) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to WiFi"); 

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN, 64);
  ZeroValue = setZeroForce();
  startTime = millis();

  http.begin("http://192.168.4.1/client_power");
  http2.begin("http://192.168.4.1/client_cadence");
  http.addHeader("Content-Type", "text/plain");
  http2.addHeader("Content-Type", "text/plain");
}

void loop() 
{
  if ((WiFiMulti.run() == WL_CONNECTED)) {  
    if (scale.is_ready()) 
    {
      reading = (float) scale.read() / 100.0;

      if (reading > MAX)
      {
        MAX = reading;
        indexM = (int) millis();
      }
  
      ForceSum += abs(reading);
      count++;
      summation += (reading - previous);
      previous = reading;
  
      if (++buffCount == 20)
      {
        slope = summation / 20.0;
        if (slope < 0.0 && prev_edge > 0.0)
        {
          cadence = 60000.0 / (float) (indexM - indexO);
          if (cadence <= 150)
          {
            ForceSum = ForceSum / (float) count;
            power_sum += (((ForceSum - ZeroValue)/82.54) * (0.1047*cadence));
			      power_count++;
  
            // Web Server Stuff
            //httpRC = http.POST(String(power));
            http2RC = http2.POST(String(cadence));
            
            ForceSum = 0.0;
            count = 0;
          }
  
          indexO = indexM;
          MAX = -1000000.0;        
        }
  
        summation = 0.0;
        prev_edge = slope;
        buffCount = 0;
      }
    }
	
  	endTime = millis();
  	if (power_count > 0 && (endTime - startTime) >= 3000) 
  	{
  	  startTime = endTime;
  	  power = power_sum / (float) power_count;
  	  power_sum = 0.0;
  	  power_count = 0;
  	  httpRC = http.POST(String(power));
  	}
  }
}

float setZeroForce()
{
  float Read = 0.0, average = 0.0;
  int tCount = 0, TimeStart = millis();

  while ((millis() - TimeStart) < 1000)
  {
    if (scale.is_ready())
    {
      Read = (float) scale.read() / 100.0;
      average += reading;
      tCount++;
    }
  }

  average =  average / (float) tCount;

  return average;
}
