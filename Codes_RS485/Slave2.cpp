#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

const int RX = 4;
const int TX = 5;
const int DE_RE = 16;


const int BMP_SCK = 13;
const int BMP_MISO = 12;
const int BMP_MOSI = 11;
const int BMP_CS = 10;

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

    pinMode(DE_RE, OUTPUT);
    digitalWrite(DE_RE, LOW);

    Serial.println("Slave 0x" + String(slaveid, HEX) + " Start\n");

    if (!bmp.begin(0x76))
    {
        Serial.println("Could not find a valid BMP280 sensor , check wiring.");
        while (1) ;
    }
    
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,Adafruit_BMP280::SAMPLING_X2,Adafruit_BMP280::SAMPLING_X16,Adafruit_BMP280::FILTER_X16,Adafruit_BMP280::STANDBY_MS_500);
}

void loop()
{
    digitalWrite(DE_RE, LOW);

    while (sf.available() > 0)
    {
        byte temp = sf.read();
        Serial.println("Request[" + String(inlen) + "]: " + String(temp, DEC) + " " + String(temp, HEX));
        _inbuff[inlen] = temp;
        inlen++;
        PreviousMillis = millis();
    }

    if (inlen >= 7)
    {
        Serial.println("\ninlen = " + String(inlen));

        byte uintAddr = _inbuff[0];
        byte function = _inbuff[1];

        if (uintAddr != slaveid)
        {
            Serial.println("Address Not ok: 0x" + String(uintAddr, HEX));
            inlen = 0;
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

        uint16_t pressure = bmp.readPressure();

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

        if (CurrentMillis - PreviousMillis > Timeout)
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