#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <ModbusRTU.h>
#include <WiFi.h>
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

WiFiClient espClient;

// void callback(char *topic,byte *payload,unsigned int length);
PubSubClient mqttClient(mqttServer, mqttPort, espClient);

Adafruit_BMP280 bmp;

#define DHTPIN 4
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN,DHTTYPE);

ModbusRTU mb;

const uint16_t REG_TEMPERATURE=100;
const uint16_t REG_HUMIDITY=101;
const uint16_t REG_PRESSURE=102;
const uint16_t REG_CURRENT=103;
const uint16_t REG_GAS=104;

const int ACS712_PIN=36;
const int MQ135_PIN=A0;

void setup(){
  Serial.begin(115200);
    Wire.begin();
    Serial.println("\Welcome to Our IoT Project!\n");
    setup_wifi();
    mqtt_connect();

    dht.begin();

    if(!bmp.begin(0x76)){
      Serial.println("Could not find a valid Sensor, check connections!");
      while(1)
        ;
    }

    mb.begin(&Serial);
    mb.slave(1);

    pinMode(ACS712_PIN,INPUT);
    pinMode(MQ135_PIN,INPUT);
}

void loop(){
  sensors_event_t event;

  dht.temperature().getEvent(&event);
  float temperature=event.temperature;

  dht.humidity().getEvent(&event);
  float humidity=event.relative_humidity;

  float pressure=bmp.readPressure()/100.0F;
  float current=readACS712();
  float gas=readMQ135();

  mb.Hreg(REG_TEMPERATURE, (uint16_t)(temperature*100));
  mb.Hreg(REG_HUMIDITY, (uint16_t)(humidity*100));
  mb.Hreg(REG_PRESSURE, (uint16_t)(pressure*100));
  mb.Hreg(REG_CURRENT, (uint16_t)(current*1000));
  mb.Hreg(REG_GAS, (uint16_t)(gas*100));

  mb.task();

  publishSensorData(temperature, humidity, pressure, current, gas);

  if(!mqttClient.loop()){
    mqtt_connect();
  }

  delay(1000);
}

float readACS712(){
  int sensorValue=analogRead(ACS712_PIN);
  float voltage=sensorValue*(5.0/4095.0);
  float current=(voltage-2.5)/0.185;

  return current;
}

float readMQ135(){
  int sensorValue=analogRead(MQ135_PIN);
  float voltage=sensorValue*(5.0/4095.0);
  float gas=(voltage-0.2)*1000;

  return gas;
}

void setup_wifi(){
  Serial.println("Connecting to WiFi network...");

  WiFi.begin(ssid,password);

  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
  }

  Serial.println("WiFi Connected!");

}

void mqtt_connect(){
  while(!mqttClient.connected()){
      Serial.println("Connecting to MQTT...");

      if(mqttClient.connect(mqttClientId,mqttUser,mqttPassword)){
        Serial.println("MQTT client Connected!");

        mqtt_subscribe(topicSubscribe);
      }else {
        Serial.println("Failed to connect, rc=");
        Serial.println(mqttClient.state());
        Serial.println("Retrying in 5 seconds");

        delay(5000);
      }
  }
}

void mqtt_subscribe(const char*topic){
  if(mqttClient.subscribe(topic)){
    Serial.println("Subscribe \"" + String(topic) + "\" Ok");
  }else {
    Serial.println("Subscribe \"" + String(topic) + "\" Failed!");
  }
}

void publishSensorData(float temperature, float humidity, float pressure, float current,float gas){
  DynamicJsonDocument doc(1024);
  doc["device_id"]=deviceId;

  JsonObject data=doc.createNestedObject("data");

  data["temperature"]=temperature;
  data["humidity"]=humidity;
  data["pressure"]=pressure;
  data["current"]=current;
  data["gas"]=gas;

  String payload;
  serializeJson(doc,payload);

  mqtt_connect();
  Serial.println("Publishing data...");
  mqttClient.publish(topicPublish,payload.c_str());
}
