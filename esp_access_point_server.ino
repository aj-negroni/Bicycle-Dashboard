#include <ESP8266WiFi.h>
#include "ESPAsyncWebServer.h"

#include <Wire.h>
#include "SparkFunMPL3115A2.h"

MPL3115A2 myPressure;

float velocity = 0.0;
float distance = 0.0;
float temperature = 0.0;
float elevation = 0.0;
float power = 0.0;
float cadence = 0.0;

/*float average_power = 0.0;
float power_sum = 0.0;
int power_count = 0;
float average_cadence = 0.0;
float cadence_sum = 0.0;
int cadence_count = 0;
float average_velocity = 0.0;
int velocity_count = 0;*/

int ReadHall = HIGH, prevHall = HIGH;
int startTime = 0, endTime = 0, interval = 0;

AsyncWebServer server(80);

/*String get_velocity() {
  if (velocity_count == 0) {
    return String(0.0);
  }
  
  average_velocity = velocity / (float) velocity_count;
  velocity = 0.0;
  velocity_count = 0;
  return String(average_velocity);
}*/

String get_elevation() {
  elevation = myPressure.readAltitudeFt();
  return String(elevation);
}

String get_temperature() {
  temperature = myPressure.readTempF();
  return String(temperature);
}

String get_average_power() {  
  /*average_power = power_sum / (float) power_count;
  power_sum = 0.0;
  power_count = 0;

  if (average_power < 20.0) {
    return String(0.0);
  }
  
  return String(average_power);*/
  if (power < 15.0) {
	return String(0.0);
  }
  
  return String(power);
}

String get_average_cadence() {
  /*if (cadence_count == 0) {
    return String(0.0);
  }
  
  average_cadence = cadence_sum / (float) cadence_count;
  cadence_sum = 0.0;
  cadence_count = 0;

  if (average_cadence < 10.0 || average_cadence > 160) {
    return String(0.0);
  }
  
  return String(average_cadence);*/
  if (cadence < 15.0) {
	  return String(0.0);
  }
  
  return String(cadence);
}

void setup()
{
  //Serial.begin(115200);
  //Serial.println();
  //Serial.print("Setting soft-AP ... ");
  WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP");

  IPAddress espIP = WiFi.softAPIP();
  //Serial.print("IP address: ");
  //Serial.println(espIP);

  server.on(
    "/client_power",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      power = atof((char*) data);
      //power_sum += power;
      //power_count++;
 
      request->send(200);
  });

  server.on(
    "/client_cadence",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {

      cadence = atof((char*) data);      
      //cadence_sum += cadence;
      //cadence_count++;
 
      request->send(200);
  });

  server.on("/velo", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(velocity).c_str());
  });

  server.on("/dist", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String((0.001214*distance)).c_str());
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", get_temperature().c_str());
  });

  server.on("/elevation", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", get_elevation().c_str());
  });

  server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", get_average_power().c_str());
  });

  server.on("/cadence", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", get_average_cadence().c_str());
  });

  // Setting up the barometer
  Wire.begin(0, 2);
  myPressure.begin();
  myPressure.setModeAltimeter();
  myPressure.setOversampleRate(7);
  myPressure.enableEventFlags();

  // Setting up the Hall Effect Sensor
  pinMode(3, FUNCTION_3);
  pinMode(3, INPUT);
  startTime = millis();
  
  server.begin();
}

void loop()
{
  //Serial.printf("Stations connected = %d\n", WiFi.softAPgetStationNum());
  ReadHall = digitalRead(3);
  endTime = millis();
  interval = endTime - startTime;

  if (ReadHall == LOW && prevHall == HIGH && interval >= 75)
  { // Wheel completed one full rotation
    startTime = endTime;
    distance++;
    velocity = 4370.01 / (float) interval;
  }
  else if (interval > 1092)
  {
    velocity = 0.0;
  }

  prevHall = ReadHall;  
}
