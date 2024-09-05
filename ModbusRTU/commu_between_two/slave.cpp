#include <ModbusRTU.h>

#define RX 13
#define TX 12
#define HREG 1
#define SLAVE_ID 1

ModbusRTU mb;

uint16_t write_value = 100;

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializing Modbus on Serial2 
    mb.slave(SLAVE_ID);
    mb.addHreg(HREG,write_value);
}



void loop()
{
    mb.task(); 
    mb.Hreg(HREG,write_value);
    

    delay(1000);
}