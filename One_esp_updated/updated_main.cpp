#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <WiFi.h>
#include <ModbusRTU.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

const char* ssid="SSID";
const char* password="Password";

String deviceId="device0001";
const char* mqttServer="192.168.1.100";
const int mqttPort=1883;
const char* mqttUser="device0001";
const char* mqttPassword="device0001";
const char* mqttClientId="device_device0001";
const char* topicPublish="Exploration-G-6";
const char* topicSubscribe="Exploration-G-6";
