#include <ModbusRTU.h>

#define RX 13
#define TX 12
#define HREG 1
#define SLAVE_ID 1

ModbusRTU mb;

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializing Modbus on Serial2 
    mb.master();
}

uint16_t write_value = 100;

void loop()
{
    mb.task(); // The magic function which handles all the communication

    if (!mb.slave())
    {
        mb.writeHreg(SLAVE_ID, HREG, write_value, nullptr);
        Serial.print("Writing the value : ");
        Serial.println(write_value);
    }

    delay(100);
}