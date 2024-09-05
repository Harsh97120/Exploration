#include <ModbusRTU.h>

#define RX 13
#define TX 12
#define HREG 1
#define SLAVE_ID 1

ModbusRTU mb;

uint16_t values[4] = {100, 200, 300, 400};

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializing Modbus on Serial2
    mb.slave(SLAVE_ID);

    for (int i = 0; i < 4; i++)
    {
        mb.addHreg(i, values[i]); // Register addresses 0, 1, 2, and 3
    }
}

void loop()
{
    mb.task();

    for (int i = 0; i < 4; i++)
    {
        mb.Hreg(i, values[i]);
    }

    delay(1000);
}