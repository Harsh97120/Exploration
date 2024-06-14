#include <ModbusRTU.h>
#include <SoftwareSerial.h>

const int RX = 16 ;
const int TX = 17 ;

const int ACS_PIN = 34;
const float ACS_SCALE = 0.185 ;

bool cbReadCurrent (TRegister * reg , uint16_t val)
{
    int sensorValue = analogRead(ACS_PIN);
    float voltage = sensorValue * (3.3 / 4095.0);
    float current = (voltage - 2.5)/ACS_SCALE;

    reg -> value = (uint16_t)(current * 100);

    return true;
}

void setup()
{
    Serial.begin(115200);
    RS485Serial.begin(9600);

    mb.begin(&RS485Serial);
    mb.slave(3);

    mb.addHreg(0,0);
    mb.onGetHreg(0,cbReadCurrent,0);

    analogReadResolution(12);
}

void loop()
{
    mb.task();
    delay(100);
}