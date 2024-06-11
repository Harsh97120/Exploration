#include <ModbusRTU.h>
#include <DHT.h>
#include <SoftwareSerial.h>

const int RX_PIN = 16;
const int TX_PIN = 17 ;
const int DHT_PIN = 4;
const char * DHT_TYPE = DHT22;

ModbusRTu mb ;
SoftwareSerial RS485Serial(RX_PIN,TX_PIN);

DHT dht(DHT_PIN,DHT_TYPE);

bool cbReadtemp(TRegister* reg,uint16_t val)
{
    dht.begin();
    reg -> value = (uint16_t)(dht.readTemperature()*10);
    return true ;
}

bool cbReadhum(TRegister* reg,uint16_t val)
{
    dht.begin();
    reg -> value = (uint16_t)(dht.readHumidity()*10);
    return true ;
}

void setup()
{
    Serial.begin(115200);
    RS485Serial.begin(9600);

    mb.begin(&RS485Serial);
    mb.slave(1);

    mb.addHreg(0,0);
    mb.addHreg(1,0);

    mb.onGetHreg(0,cbReadtemp,0);
    mb.onGetHreg(0,cbReadhum,0);

    dht.begin();
}

void loop()
{
    mb.task();
    delay(100);
}
