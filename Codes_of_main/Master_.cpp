#include <ModbusRTU.h>
#include <SoftwareSerial.h>

const int RX_PIN = 16;
const int TX_PIN = 17;

ModbusRTU mb ;
SoftwareSerial RS485Serial(RX_PIN,TX_PIN);

const uint8_t slave1 = 1;
const uint8_t slave2 = 2;
const uint8_t slave3 = 3;

float temperature = 0 ;
float humidity = 0;
float pressure = 0 ;
float current = 0;

bool readSensorData(uint8_t slaveID, uint16_t reg , uint16_t * result)
{
    if(mb.readHreg(slaveID,reg,result,1))
    {
        while(mb.slave())
        {
            mb.task();
        }

        return true;
    }

    return false ;
}

void setup()
{
    Serial.begin(115200);
    RS485Serial.beign(9600);

    mb.begin(&RS485Serial);
}


void loop()
{
    unit16_t result ;

    if(readSensorData(slave1,0,&result))
    {
        temperature = result / 10.0;
    }

    if(readSensorData(slave1,1,&result))
    {
        humidity = result / 10.0;
    }

    if(readSensorData(slave2,0,&result))
    {
        pressure = result / 10.0;
    }

    if(readSensorData(slave3,0,&result))
    {
        current = result / 10.0;
    }

    Serial.print("T: ");
    Serial.print(temperature);
    Serial.print(" C, H: ");
    Serial.print(humidity);
    Serial.print(" %, P: ");
    Serial.print(pressure);
    Serial.print(" hPa, I: ");
    Serial.print(current);
    Serial.println(" A");

    delay(2000);

}