#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

const int RX = 4;
const int TX = 5;
const int DE_RE = 16;

const int I2C_SDA = 21;
const int I2C_SCL = 22;

SoftwareSerial sf(RX, TX);
Adafruit_BMP280 bmp;

byte inlen, _inbuff[8] = {0}, outlen, _outbuff[7] = {0};
byte slaveid = 0x02, functionCode = 0x03;

unsigned long CurrentMillis, PreviousMillis, Timeout = (unsigned long)10;

void setup()
{
    Serial.begin(115200);
    sf.begin(9600);
    sf.flush();

    Wire.begin(I2C_SDA, I2C_SCL);

    pinMode(DE_RE, OUTPUT);
    digitalWrite(DE_RE, LOW);

    Serial.println("Slave 0x" + String(slaveid, HEX) + " Start\n");

    if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor , check wiring.");
        while (1)
            ;
    }
}

void loop()
{
    digitalWrite(DE_RE, LOW);

    while (sf.available() > 0)
    {
        byte temp = sf.read();
        Serial.println("Request[" + String(inlen) + "]: " + Strinh(temp, DEC) + " " + String(temp, HEX));
        _inbuff[inlen] = temp;
        inlen++;
        PreviousMillis = millis();
    }

    if (inlen >= 7)
    {
        Serial.println("\ninlen = " + String(inlen));

        byte unitAddr = _inbuff[0];
        byte function = _inbuff[1];

        if (unitAddr != slaveid)
        {
            Serial.println("Address Not ok: 0x" + String(uintAddr, HEX));
            inLen = 0;
            sf.flush();
            return;
        }

        Serial.println("Address ok: 0x" + String(uintAddr, HEX));

        if (function != functionCode)
        {
            Serial.println("Function code not ok: 0x" + String(function, HEX));
            inlen = 0;
            sf.flush();

            return;
        }

        Serial.println("Function code ok: 0x" + String(function, HEX));

        byte crc[2];

        uint16_t u16CRC = 0xFFFF;

        for (byte i = 0; i < inlen - 2; i++)
        {
            u16CRC = crc16_update(u16CRC, _inbuff[i]);
        }

        crc[0] = lowByte(u16CRC);
        crc[1] = highByte(u16CRC);

        if (_inbuff[inlen - 2] != crc[0] || _inbuff[inlen - 1] != crc[1])
        {
            Serial.println("CRC not ok: 0x" + String(u16CRC, HEX));
            inlen = 0;
            sf.flush();

            return;
        }

        Serial.println("CRC ok: 0x" + String(u16CRC, HEX));

        digitalWrite(DE_RE, HIGH);

        sf.flush();
        inlen = 0;

        float pressure = bmp.readPressure();

        Serial.println("\nPressure = " + String(pressure) + " 0x" + String(pressure, HEX) + "\n\n");

        outlen = 2;

        _outbuff[0] = slaveid;
        _outbuff[1] = functionCode;
        _outbuff[2] = outlen;
        _outbuff[3] = highByte(pressure);
        _outbuff[4] = lowByte(pressure);

        u16CRC = 0xFFFF;

        for (byte i = 0; i < 5; i++)
        {
            u16CRC = crc16_update(u16CRC, _outbuff[i]);
        }

        _outbuff[5] = lowByte(u16CRC);
        _outbuff[6] = highByte(u16CRC);

        Serial.write(_outbuff, sizeof(_outbuff));
        Serial.println("\n");
    }
    else if (inlen > 0)
    {
        CurrentMillis = millis();

        if (CurrentMilli - PreviouMillis > Timeout)
        {
            PreviousMillis = CurrentMillis;
            inlen = 0;
            Serial.println("\nTimeout\n");
        }
    }
}

static uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    crc ^= a;
    for (int i = 0; i < 8; ++i)
    {
        if (crc & 1)
        {
            crc = (crc >> 1) ^ 0xA001;
        }
        else
        {
            crc = (crc >> 1);
        }
    }
    return crc;
}