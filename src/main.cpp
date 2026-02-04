#include "wifi_manager.h"

#define DEBUG

const int LED_pins = 26;
const int Sensor_pin = 32;

const int QualityLimit = 1500;
const int deviationLimit = 250;

DeviceServer wifi(80);

void setup() {
  Serial.begin(115200);
  
  #ifdef DEBUG
  Serial.println("Initializing Smoke Detector...");
  delay(1000);
  Serial.println("Initialization Complete.");
  Serial.println("running...");
  #endif
  
  (wifi.begin(Sensor_pin))? Serial.println("WiFi started successfully!") : Serial.println("Failed to start WiFi!");

  pinMode(LED_pins, OUTPUT);
  pinMode(Sensor_pin, INPUT);
}

void loop() {
  wifi.handle();
  static int prev_sensorValue = 0;
  
  int sensorValue = analogRead(Sensor_pin);
  int delta = abs(sensorValue - prev_sensorValue);
  prev_sensorValue = sensorValue;
  
  bool airQuality = (sensorValue < QualityLimit && delta < deviationLimit);
  
  #ifdef DEBUG
  Serial.println("-----------------------");
  Serial.print("Sensor Value: ");
  Serial.println(sensorValue);
  Serial.print("Delta: ");
  Serial.println(delta);
  Serial.print("Air Quality: ");
  (airQuality)? Serial.println("Good") : Serial.println("Poor");
  Serial.println("-----------------------\n");
  #endif
  
  if (airQuality) digitalWrite(LED_pins, LOW);
  else digitalWrite(LED_pins, HIGH);
  
  delay(1000);
}