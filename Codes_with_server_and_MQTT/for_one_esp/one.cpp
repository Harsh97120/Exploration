#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ModbusRTU.h>
#include <Wifi.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>

// Wifi

const char *ssid = "Your SSID";
const char *password = "Your Password";

// MQTT

String device_id = "Device0001";
const char *mqtt_server = "mqtt://localhost:1883";
const int mqtt_port = 1883;
const char *mqtt_user = "Device0001";
const char *mqtt_password = "Device0001";
const char *mqtt_clientId = "Deivce_Device0001";
const char *topic_publish = "JPLearning_SensorData";
const char *topic_subscribe = "JPLearning_CommandRequest";
bool read_success = true;

#define DHTPIN 4
#define DHTTYPE DHT22
DHT_Unified dht(DHTPIN, DHTTYPE);

WifiClient esp_client;
void callback(char *topic, bye *payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, callback, esp_client);

HTTPClient http;

// Define I2C addresses
Adafruit_BMP280 bmp;    // I2C
Adafruit_INA219 ina219; // I2C

// Modbus server
ModbusRTU mb;

// Modbus register addresses
const uint16_t REG_TEMPERATURE = 100;
const uint16_t REG_HUMIDITY = 101;
const uint16_t REG_PRESSURE = 102;
const uint16_t REG_VOLTAGE = 103;
const uint16_t REG_CURRENT = 104;

void setup()
{
    Serial.begin(115200);
    Wire.begin();
    Serial.println("\nWelcome to IoT project.\n");
    setup_wifi();
    get_lastEvent();
    mqtt_connect();

    // Initialize DHT22 sensor
    dht.begin();

    // Initialize BMP280 sensor
    if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        while (1)
            ;
    }

    // Initialize INA219 sensor
    if (!ina219.begin())
    {
        Serial.println("Failed to find INA219 chip");
        while (1)
            ;
    }

    // Initialize Modbus server with address 1
    mb.begin(&Serial);
    mb.slave(1);
}

void loop()
{
    // Read DHT22 sensor data
    sensors_event_t event;
    dht.temperature().getEvent(&event);
    float temperature = event.temperature;
    dht.humidity().getEvent(&event);
    float humidity = event.relative_humidity;

    // Read BMP280 sensor data
    float pressure = bmp.readPressure() / 100.0F; // Convert to hPa

    // Read INA219 sensor data
    float voltage = ina219.getBusVoltage_V();
    float current = ina219.getCurrent_mA();

    read_success = true;

    if (isnan(temperature) || isnan(humidity))
    {
        Serial.println("\n\nFailed to read from DHT22 sensor!");
        read_success = false;
        delay(1000);
    }

    if (isnan(pressure))
    {
        Serial.println("\n\nFailed to read from bmp280 sensor!");
        read_success = false;
        delay(1000);
    }

    if (isnan(voltage) || isnan(current))
    {
        Serial.println("\n\nFailed to read from ina219 sensor!");
        read_success = false;
        delay(1000);
    }

    if (read_success)
    {
        String pkt = "{";
        pkt += "\"device_id\": \"" + device_id + "\", ";
        pkt += "\"type\": \"Temperature\", ";
        pkt += "\"value\": " + String(temperature) + "";
        pkt += "}";
        mqtt_publish((char *)pkt.c_str());

        String pkt2 = "{";
        pkt2 += "\"device_id\": \"" + device_id + "\", ";
        pkt2 += "\"type\": \"Humidity\", ";
        pkt2 += "\"value\": " + String(humidity) + "";
        pkt2 += "}";
        mqtt_publish((char *)pkt2.c_str());

        String pkt3 = "{";
        pkt2 += "\"device_id\": \"" + device_id + "\", ";
        pkt2 += "\"type\": \"Pressure\", ";
        pkt2 += "\"value\": " + String(pressure) + "";
        pkt2 += "}";
        mqtt_publish((char *)pkt3.c_str());

        String pkt4 = "{";
        pkt2 += "\"device_id\": \"" + device_id + "\", ";
        pkt2 += "\"type\": \"voltage\", ";
        pkt2 += "\"value\": " + String(voltage) + "";
        pkt2 += "}";
        mqtt_publish((char *)pkt4.c_str());
    }

    // Set Modbus registers
    mb.Hreg(REG_TEMPERATURE, (uint16_t)(temperature * 100)); // Store as integer
    mb.Hreg(REG_HUMIDITY, (uint16_t)(humidity * 100));       // Store as integer
    mb.Hreg(REG_PRESSURE, (uint16_t)(pressure * 100));       // Store as integer
    mb.Hreg(REG_VOLTAGE, (uint16_t)(voltage * 1000));        // Store as integer
    mb.Hreg(REG_CURRENT, (uint16_t)(current));               // Store as integer

    // Handle Modbus requests
    mb.task();

    if (!mqtt_client.loop())
    {
        mqtt_connect();
    }

    // Delay to prevent overwhelming the sensors
    delay(1000);
}

void setup_wifi()
{
    Serial.println();
    Serial.println("Connecting to ");
    Serial.println("\"" + String(ssid) + "\"");

    Wifi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        dealy(500);
        Serial.print(".");
    }

    Serial.println("\nWifi connected");
}

void mqtt_connect()
{
    while (!mqtt_client.connected())
    {
        Serial.println("Attempting MQTT connection..");

        if (mqtt_client.connect(mqtt_clienntId))
        {
            Serial.println("MQTT Client connected");

            mqtt_subscribe(topic_subscribe);
        }
        else
        {
            Serial.print("Failed , rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");

            delay(5000);
        }
    }
}

void mqtt_subscribe(const char *topic)
{
    if (mqtt_client.subscribe(topic))
    {
        Serial.println("Subscribe \"" + String(topic) + "\"ok");
    }
    else
    {
        Serial.println("Subscribe \"" + String(topic) + "\"failed");
    }
}

void mqtt_publish(char *data)
{
    mqtt_connect();
    Serial.println("Publish Topic: \"" + String(topic_publish) + "\"");
    if (mqtt_client.publish(topic_publish, data))
    {
        Serial.println("Publish \"" + String(data) + "\" ok");
    }
    else
    {
        Serial.println("Publish \"" + String(data) + "\" failed");
    }
}