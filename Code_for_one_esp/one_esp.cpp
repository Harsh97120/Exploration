#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <ModbusRTU.h>

// Define DHT sensor type and pin
#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

// Define I2C addresses
Adafruit_BMP280 bmp; // I2C
Adafruit_INA219 ina219; // I2C

// Modbus server
ModbusRTU mb;

// Modbus register addresses
const uint16_t REG_TEMPERATURE = 100;
const uint16_t REG_HUMIDITY = 101;
const uint16_t REG_PRESSURE = 102;
const uint16_t REG_VOLTAGE = 103;
const uint16_t REG_CURRENT = 104;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  
  // Initialize DHT22 sensor
  dht.begin();
  
  // Initialize BMP280 sensor
  if (!bmp.begin(0x76)) {
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
  
  // Initialize INA219 sensor
  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1);
  }
  
  // Initialize Modbus server with address 1
  mb.begin(&Serial);
  mb.slave(1);
}

void loop() {
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
  
  // Set Modbus registers
  mb.Hreg(REG_TEMPERATURE, (uint16_t)(temperature * 100)); // Store as integer
  mb.Hreg(REG_HUMIDITY, (uint16_t)(humidity * 100)); // Store as integer
  mb.Hreg(REG_PRESSURE, (uint16_t)(pressure * 100)); // Store as integer
  mb.Hreg(REG_VOLTAGE, (uint16_t)(voltage * 1000)); // Store as integer
  mb.Hreg(REG_CURRENT, (uint16_t)(current)); // Store as integer

  // Handle Modbus requests
  mb.task();

  // Delay to prevent overwhelming the sensors
  delay(1000);
}
