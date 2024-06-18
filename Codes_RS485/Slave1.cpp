#include <SoftwareSerial.h>
#include <DHT.h>

const int RX = 4;
const int TX = 5;
const int DE_RE = 16;
const int DHT_PIN = 7;

const char *DHT_TYPE = DHT22;

SoftwareSerial sf(RX, TX);
DHT dht(DHT_PIN, DHT_TYPE);

byte inLen, _inbuff[9] = {0}, outLen, _outbuff[9] = {0};
byte slaveID = 0x01, functionCode = 0x03;

unsigned long CurrentMillis, PreviousMillis, Timeout = (unsigned long)10;

void setup()
{
    Serial.begin(115200);
    sf.begin(9600);
    sf.flush();

    pinMode(DE_RE, OUTPUT);
    digitalWrite(DE_RE, LOW);

    dht.begin();

    Serial.println("Slave 0x" + String(slaveID, HEX) + " Start\n");
}

void loop()
{

    digitalWrite(DE_RE, LOW);

    while (sf.available() > 0)
    {
        byte temp = sf.read();
        Serial.println("Request[" + String(inLen) + "]: " + String(temp, DEC) + " " + String(temp, HEX));
        _inbuff[inLen] = temp;
        inLen++;
        PreviousMillis = millis();
    }

    if (inLen >= 7)
    {
        Serial.println("\ninLen = " + String(inLen));

        byte uintAddr = _inbuff[0];
        byte function = _inbuff[1];

        if (uintAddr != slaveID)
        {
            Serial.println("Address Not Ok: 0x" + String(uintAddr, HEX));
            inLen = 0;
            sf.flush();
            return;
        }

        Serial.println("AddressOk: 0x" + String(uintAddr, HEX));

        if (function != functionCode)
        {
            Serial.println("Function Code Not ok: 0x" + String(function, HEX));
            inLen = 0;
            sf.flush();
            return;
        }

        Serial.println("Function Code 0k: 0x" + String(function, HEX));

        byte crc[2];

        uint16_t u16CRC = 0xFFFF;

        for (byte i = 0; i < inLen - 2; i++)
        {
            u16CRC = crc16_update(u16CRC, _inbuff[i]);
        }

        crc[0] = lowByte(u16CRC);
        crc[1] = highByte(u16CRC);

        if (_inbuff[inLen - 2] != crc[0] || _inbuff[inLen - 1] != crc[1])
        {
            Serial.println("CRC Not OK: 0x" + String(u16CRC, HEX));
            inLen = 0;
            sf.flush();
            return;
        }

        Serial.println("CRC OK: 0x" + String(u16CRC, HEX));

        digitalWrite(DE_RE, HIGH);
        sf.flush();

        inLen = 0;

        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();

        Serial.println("\nHumidity: " + String(humidity) + "0x" + String(humidity, HEX) + "\n\n");
        Serial.println("\nTemperature: " + String(temperature) + "0x" + String(temperature, HEX) + "\n\n");

        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println("Failed to read DHT sensor!");
            return;
        }

        outLen = 4;

        _outbuff[0] = slaveID;
        _outbuff[1] = functionCode;
        _outbuff[2] = outLen;
        _outbuff[3] = highByte((int)temperature);
        _outbuff[4] = lowByte((int)temperature);
        _outbuff[5] = highByte((int)humidity);
        _outbuff[6] = lowByte((int)humidity);

        u16CRC = 0xFFFF;

        for (byte i = 0; i < 5; i++)
        {
            u16CRC = crc16_update(u16CRC, _outbuff[i]);
        }

        _outbuff[7] = lowByte(u16CRC);
        _outbuff[8] = highByte(u16CRC);

        sf.write(_outbuff, sizeof(_outbuff));
        Serial.println("\n");
    }
    else if (inLen > 0)
    {
        CurrentMillis = millis();

        if (CurrentMillis - PreviousMillis > Timeout)
        {
            PreviousMillis = CurrentMillis;
            inLen = 0;
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

int hexToDec(String hexString)
{
    int decValue = 0, nextInt;
    for (int i = 0; i < hexString.length(); i++)
    {
        nextInt = int(hexString.charAt(i));
        if (nextInt >= 48 && nextInt <= 57)
            nextInt = map(nextInt, 48, 57, 0, 9);
        if (nextInt >= 65 && nextInt <= 70)
            nextInt = map(nextInt, 65, 70, 10, 15);
        if (nextInt >= 97 && nextInt <= 102)
            nextInt = map(nextInt, 97, 102, 10, 15);
        nextInt = constrain(nextInt, 0, 15);
        decValue = (decValue * 16) + nextInt;
    }
    return decValue;
}