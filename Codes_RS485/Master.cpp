#include <SoftwareSerial.h>
#include <ModbusMaster.h>

const int RX = 4;
const int TX = 5;
const int DE_RE = 16;

SoftwareSerial sf(RX, TX);

ModbusMaster node;

#define Reg_Address 0x0000

byte SlaveID[] = {0x01, 0x02, 0x03};

long values[4];

void preTransmission()
{
    digitalWrite(DE_RE, HIGH);
}

void postTransmission()
{
    digitalWrite(DE_RE, LOW);
}

void setup()
{
    Serial.begin(115200);
    sf.begin(9600);

    pinMode(DE_RE, OUTPUT);

    Serial.println("Master Start.\n");
}

void loop()
{
    Read_Data();
    Print_Data();

    delay(5000);
}

long Read_Data()
{
    byte i = 0, index_for_val = 0 ;
    for (; index_for_val < 4, i < 3; i++)
    {
        node.begin(SlaveID[i], sf);

        uint8_t result;

        byte dataReadLength = 1;

        if (i == 0)
        {
            dataReadLength = 2;
        }

        uint16_t data[dataReadLength];

        result = node.readHoldingRegisters(Reg_Address, dataReadLength);

        if (result == node.ku8MBSuccess)
        {
            for (byte j = 0; j < dataReadLength; j++)
            {
                data[j] = (node.getResponseBuffer(j));
            }

            String dataTemp = String(data[0], HEX);
            Serial.println("\n\ndataTemp = " + dataTemp);

            values[index_for_val] = hexToDec(dataTemp);
            index_for_val++;

            if (i == 0)
            {
                String dataTemp = String(data[1], HEX);
                Serial.println("\n\ndataTemp = " + dataTemp);

                values[index_for_val] = hexToDec(dataTemp);
                index_for_val++;
            }
        }
        else
        {
            Serial.println("Connect modbus fail. SlaveID: 0x" + String(SlaveID[i], HEX) + " and Register: 0x" + String(Reg_Address, HEX));
            delay(100);
        }
    }
}

void Print_Data()
{
    Serial.println("Temperature: ");
    Serial.print(values[0]);

    Serial.println("\nHumidity: ");
    Serial.print(values[1]);

    Serial.println("\nPressure: ");
    Serial.print(values[2]);

    Serial.println("\nCurrent: ");
    Serial.print(values[3]);
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