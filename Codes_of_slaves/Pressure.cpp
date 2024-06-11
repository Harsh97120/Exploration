#include <ModbusRTU.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

const int RX = 16;
const int TX = 17;

ModbusRTU mb;
SoftwareSerial RS485Serial(RX,TX);
Adafruit_BMP280 bmp ;

bool cbReadPressure(TRegister * reg , uint16_t val)
{
    reg -> value = (uint16_t)(bmp.readPressure()/100.0);
    return true;
}

void setup()
{
    Serial.begin(115200);
    RS485Serial.begin(9600);

    mb.begin(&RS485Serial);
    mb.slave(2);

    mb.addHreg(0,0);
    mb.onGetHreg(0,cbReadPressure,0);

    if(!bmp.begin())
    {
        Serial.println("COuld not find a valid BMP280 sensor,check wiring!");
        while(1);
    }
}

void loop()
{
    mb.task();
    delay(100);
}

