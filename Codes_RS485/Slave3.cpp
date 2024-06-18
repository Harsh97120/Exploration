#include <SoftwareSerial.h>
#include <Adafruit_INA219.h>
#include <Wire.h>


const int RX = 4;
const int TX = 5;
const int DE_RE_PIN = 16;


SoftwareSerial Soft_Serial(RX, TX);
Adafruit_INA219 ina219 ;

byte inLen, _inBuff[8] = {0}, outLen, _outBuff[9] = {0};
byte slaveID = 0x02, functionCode = 0x03;

unsigned long CurrentMillis, PreviousMillis, Timeout = (unsigned long)10;

void setup()
{
    Serial.begin(115200);
    Soft_Serial.begin(9600);
    Soft_Serial.flush(); // flush transmit buffer

    pinMode(DE_RE_PIN  , OUTPUT);   
    digitalWrite(DE_RE_PIN , LOW); 

    //Wire.begin(SDA ,SCL);
    
    Serial.println("Slave 0x" + String(slaveID, HEX) + " Start\n");

    if(!ina219.begin())
    {
        Serial.println("Failed to find INA219 chip.");
        while(1)
        {
            delay(10);
        }
    }

}
void loop()
{
    
    digitalWrite(DE_RE_PIN, LOW); 

    while (Soft_Serial.available() > 0)
    {
        byte temp = Soft_Serial.read();
        Serial.println("Request[" + String(inLen) + "]: " + String(temp, DEC) + " " + String(temp, HEX));
        _inBuff[inLen] = temp;
        inLen++;
        PreviousMillis = millis();
    }

    if (inLen >= 7)
    {
        Serial.println("\ninLen = " + String(inLen));
        byte unitAddr = _inBuff[0];
        byte function = _inBuff[1];

        if (unitAddr != slaveID)
        {
            Serial.println("Address Not OK: 0x" + String(unitAddr, HEX));
            inLen = 0;
            Soft_Serial.flush(); // flush transmit buffer
            return;
        }
        Serial.println("Address OK: 0x" + String(unitAddr, HEX));

        if (function != functionCode)
        {
            Serial.println("Function Code Not OK: 0x" + String(function, HEX));
            inLen = 0;
            Soft_Serial.flush(); // flush transmit buffer
            return;
        }

        Serial.println("Function Code OK: 0x" + String(unitAddr, HEX));

        byte crc[2];

        uint16_t u16CRC = 0xFFFF;
        for (byte i = 0; i < inLen - 2; i++)
        {
            u16CRC = crc16_update(u16CRC, _inBuff[i]);
        }
        crc[0] = lowByte(u16CRC);
        crc[1] = highByte(u16CRC);

        if (_inBuff[inLen - 2] != crc[0] || _inBuff[inLen - 1] != crc[1])
        {
            Serial.println("CRC Not OK: 0x" + String(u16CRC, HEX));
            inLen = 0;
            Soft_Serial.flush(); 
            return;
        }
        Serial.println("CRC OK: 0x" + String(u16CRC, HEX));

        digitalWrite(DE_RE_PIN, HIGH); 
        Soft_Serial.flush();          
        inLen = 0;

        uint16_t busvoltage = ina219.getBusVoltage_V();
        uint16_t current = ina219.getCurrent_mA();
        
        Serial.println("\nVoltage: " + String(busvoltage) + " 0x" + String(busvoltage, HEX) + "\n\n");
        Serial.println("\nCurrent: " + String(current) + " 0x" + String(current, HEX) + "\n\n");
        

        outLen = 4;
        
        _outBuff[0] = slaveID;
        _outBuff[1] = functionCode;
        _outBuff[2] = outLen;
        _outBuff[3] = highByte(busvoltage);
        _outBuff[4] = lowByte(busvoltage);
        _outBuff[5] = highByte(current);
        _outBuff[6] = lowByte(current);

        
        u16CRC = 0xFFFF;
        for (byte i = 0; i < 5; i++)
        {
            u16CRC = crc16_update(u16CRC, _outBuff[i]);
        }
        

        _outBuff[7] = lowByte(u16CRC);
        _outBuff[8] = highByte(u16CRC);

        Soft_Serial.write(_outBuff, sizeof(_outBuff));
        Serial.println("\n");
    }
    else if (inLen > 0)
    {
        CurrentMillis = millis();
        if (CurrentMillis - PreviousMillis > Timeout)
        {
            PreviousMillis = CurrentMillis;
            inLen = 0;
            Serial.println("\nTimout\n");
        }
    }
}
static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    crc ^= a;
    for (int i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }
    return crc;
}