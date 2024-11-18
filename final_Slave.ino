#include <ModbusRTU.h>
#include "DHT.h"
#include <SoftwareSerial.h>

#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Wire.h>


#define RX 13
#define TX 12
#define SLAVE_ID 1
#define HREG1 1
#define HREG2 2
#define HREG3 3
#define HREG4 4
#define HREG5 5
#define HREG6 6
#define HREG7 7
#define HREG8 8
#define HREG9 9
#define HREG10 10
#define HREG11 11
#define HREG12 12

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

#define SDA_PIN 21
#define SCL_PIN 22 

#define DHTPIN 14
#define DHTTYPE DHT22

#define MQ135_PIN 34

#define ACS_PIN 25

const int LED = 2;

DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP280 bmp;

ModbusRTU mb;

uint16_t values[] = { 5, 8, 5, 0 };
uint16_t addr[] = { 1, 2, 3, 4 };

bool temp1 = false;
bool temp2 = false;
bool hum1 = false;
bool hum2 = false;
bool pressure1 = false;
bool pressure2 = false;
bool ppm1 = false ;
bool ppm2 = false ;
bool current1 = false ;
bool current2 = false ;
bool voltage1 = false ;
bool voltage2 = false ;

float temperature = 0;
float humidity = 0;
float RZERO = 76.63;
float SCALING_FACTOR = 10.0; 
float ACS_SCALE = 0.185; 

void setup() {
  Serial.begin(9600, SERIAL_8N1);
  Serial2.begin(9600, SERIAL_8N1, RX, TX);
  pinMode(LED, OUTPUT);
  mb.begin(&Serial2);  //Initializes Modbus on Serial2 Channel
  mb.master();
  dht.begin();
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor , check wiring.");
    } 
}

void loop() {
  mb.task();

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read DHT");
    delay(2000);
    return;
  }

  int temp_before_dec = (int)(temperature);
  int temp_after_dec = (temperature - temp_before_dec) * 100;

  int hum_before_dec = (int)(humidity);
  int hum_after_dec = (humidity - hum_before_dec) * 100;

  int pressure_before_dec = 0;
  int pressure_after_dec = 0 ;

  if (!bmp.begin(0x76)) {  
    Serial.println("Could not find a valid BMP280 sensor, check wiring!"); 
    delay(1000);
  }
  else 
  {
    float pressure = bmp.readPressure() / 1000.0F;
    pressure_before_dec = (int)(pressure);
    pressure_after_dec = (pressure -pressure_before_dec) * 100;
  }

  int rawValue = analogRead(MQ135_PIN);
  float voltage1 = rawValue * (3.3 / 4095.0);
  float ppm = (voltage1 / SCALING_FACTOR) * 1000;
  int ppm_before_dec = (int)(ppm);
  int ppm_after_dec = (ppm -ppm_before_dec) * 100 ;

  // int ppm_before_dec = random(0, 50);
  // int ppm_after_dec = random(0, 50); 

  int sensorValue = analogRead(ACS_PIN);
  float voltage = sensorValue * (3.3 / 4095.0);
  float current = (voltage - 2.5)/ACS_SCALE; 

  // int current_before_dec = (int)(current);
  // int current_after_dec = (current -current_before_dec) * 100 ; 

  // int voltage_before_dec = (int)(voltage);
  // int voltage_after_dec = (voltage - voltage_before_dec) * 100; 

  int current_before_dec = random(0, 50);
  int current_after_dec = random(0, 50) ; 

  int voltage_before_dec = random(0, 50);
  int voltage_after_dec = random(0, 50) ; 

  if (!mb.slave()) {
    if (!temp1) {
      mb.writeHreg(SLAVE_ID, HREG1, temp_before_dec, cbWriteGreen);
      delay(100); 
    } else if (!temp2) {
      mb.writeHreg(SLAVE_ID, HREG2, temp_after_dec, cbWriteRed);
      delay(100);
    } else if (!hum1) {
      mb.writeHreg(SLAVE_ID, HREG3, hum_before_dec, cbHum1);
      delay(100);
    } else if (!hum2) {
      mb.writeHreg(SLAVE_ID, HREG4, hum_after_dec, cbHum2);
      delay(100);
    } else if (!pressure1) {
      mb.writeHreg(SLAVE_ID, HREG5, pressure_before_dec, cbpressure1);
      delay(100);
    } else if (!pressure2) {
      mb.writeHreg(SLAVE_ID, HREG6, pressure_after_dec, cbpressure2);
      delay(100);
    } else if (!ppm1) {
      mb.writeHreg(SLAVE_ID, HREG7, ppm_before_dec, cbppm1);
      delay(100);
    } else if (!ppm2) {
      mb.writeHreg(SLAVE_ID, HREG8, ppm_after_dec, cbppm2);
      delay(100);
    } else if (!current1) {
      mb.writeHreg(SLAVE_ID, HREG9, current_before_dec, cbcurrent1);
      delay(100);
    } else if (!current2) {
      mb.writeHreg(SLAVE_ID, HREG10, current_after_dec, cbcurrent2);
      delay(100);
    }else if (!voltage1) {
      mb.writeHreg(SLAVE_ID, HREG11, voltage_before_dec, cbvoltage1);
      delay(100);
    } else if (!voltage2) {
      mb.writeHreg(SLAVE_ID, HREG12, voltage_after_dec, cbvoltage2);
      delay(100);
    } else {
      Serial.println("All values are seted to false.");
      temp1 = temp2 = hum1 = hum2 = pressure1 = pressure2 = ppm1 = ppm2 = voltage1 = voltage2 = current1 = current2 =  false;
    }
  }

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
}




bool cbWriteGreen(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Temp1 Write successful");
    temp1 = true;
    return true;
  } else {
    Serial.print("Temp 1 Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbWriteRed(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Temp2 Write successful");
    temp2 = true;
    return true;
  } else {
    Serial.print("Temp 2 Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbHum1(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Hum1 Write successful");
    hum1 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbHum2(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Hum2 Write successful");
    hum2 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbpressure1(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Pressure1 Write successful");
    pressure1 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbpressure2(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Pressure2 Write successful");
    pressure2 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbppm1(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("PPM1 Write successful");
    ppm1 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbppm2(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("PPM2 Write successful");
    ppm2 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbcurrent1(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Current1 Write successful");
    current1 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbcurrent2(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Current2 Write successful");
    current2 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbvoltage1(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Voltage1 Write successful");
    voltage1 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}

bool cbvoltage2(Modbus::ResultCode event, uint16_t transactionId, void* data) {
  if (event == Modbus::EX_SUCCESS) {
    Serial.println("Voltage2 Write successful");
    voltage2 = true;
    return true;
  } else {
    Serial.print("Write failed with code: ");
    Serial.println(event);
    return false;
  }
}
