/*
Using serial communication to write a holding register inside the slave

RX and TX pins of one controller is connected to the opposite pin of the other controller, i.e,
RX (Master) -> TX (Slave)
TX (Master) -> RX (Slave)

Refer to comments in the code for more details
*/
#include <ModbusRTU.h>
#include "DHT.h"
#include <SPI.h>
#include <Wire.h>

#define RX 13
#define TX 12
#define HREG 1 // Holding register address
#define SLAVE_ID 1

#define DHTPIN 14
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);

ModbusRTU mb;

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializing Modbus on Serial2 Channel
    mb.master();        // Tells the current device that it is the master

    dht.begin(); // Temperature sensor starting....
}

uint16_t write_value = 100;

void loop()
{
    mb.task(); // The magic function which handles all the communication

    float temperature = dht.readTemperature();

    if (isnan(temperature))
    {
        Serial.println("Failed to read DHT");
        return;
    }

    if (!mb.slave())
    {
        mb.writeHreg(SLAVE_ID, HREG, temperature, nullptr); // Used to write into the holding register
                                                            //  ^ is the callback function, currently NULL
        Serial.print("Writing the value : ");
        temperature = temperature * 100;
        Serial.println(temperature);
    }

    delay(100);
}