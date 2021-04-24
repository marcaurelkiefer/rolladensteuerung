#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WifiManager.h>

void setup() {
  WiFiManager wiFiManager;
  wiFiManager.autoConnect("Rolladen");
  Serial.println("connected...yeey :)");
}

void loop() {
  // put your main code here, to run repeatedly:
}