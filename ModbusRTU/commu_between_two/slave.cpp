#include <ModbusRTU.h>
#include <DHT.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define RX 13
#define TX 12
#define HREG 1
#define SLAVE_ID 1
#define DHT_PIN 7

// Temperature sensor
const char *DHT_TYPE = DHT22;
DHT dht(DHT_PIN, DHT_TYPE);

// Pressure sensor
const int BMP_SCK = 13;
const int BMP_MISO = 12;
const int BMP_MOSI = 11;
const int BMP_CS = 10;
Adafruit_BMP280 bmp;

ModbusRTU mb;

uint16_t write_value = 100;

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializing Modbus on Serial2
    mb.slave(SLAVE_ID);
    mb.addHreg(HREG, write_value);

    dht.begin();
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,Adafruit_BMP280::SAMPLING_X2,Adafruit_BMP280::SAMPLING_X16,Adafruit_BMP280::FILTER_X16,Adafruit_BMP280::STANDBY_MS_500);
}

void loop()
{
    mb.task();

    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature))
    {
        Serial.println("Failed to read DHT sensor!");
        return;
    }

    if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor , check wiring.");
        return ;
    }

    float pressure

    values[0] = humidity;
    values[1] = temperature;

    for (int i = 0; i < 4; i++)
    {
        mb.Hreg(i, values[i]);
    }

    delay(1000);
}