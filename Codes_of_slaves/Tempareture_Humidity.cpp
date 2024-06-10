#include <SoftwareSerial.h>
#include <ModbusMaster.h>

// GPIO Pins
byte TX_PIN = 4, RX_PIN = 5;
byte DE_RE_PIN = 16, LED_PIN = 2;

SoftwareSerial Soft_Serial(RX_PIN, TX_PIN);

ModbusMaster node;

#define Sensor_ID 1
#define Reg_Address 0x0000  // for Humidity and Temperature

float humidity, temperature;

void preTransmission()
{
    digitalWrite(DE_RE_PIN, HIGH);
}
void postTransmission()
{
    digitalWrite(DE_RE_PIN, LOW);
}

void setup()
{
    Serial.begin(115200);
    Soft_Serial.begin(9600);

    pinMode(DE_RE_PIN, OUTPUT); // DE/RE Controling pin of RS-485
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    node.begin(Sensor_ID, Soft_Serial);
    node.preTransmission(preTransmission);
    node.postTransmission(postTransmission);

}

void loop()
{
    Read_Data();
    Serial.println("\nHumidity = " + String(humidity, 2) + " %");
    Serial.println("Temperature = " + String(temperature, 2) + " °C\n\n");
    delay(5000);
}

void Read_Data()
{
    uint8_t result, j;
    long value = 0;
    byte dataReadLength = 2;
    uint16_t data[dataReadLength];

    // Disable watchdog reset
    ESP.wdtDisable();

    result = node.readHoldingRegisters(Reg_Address, dataReadLength);

    // Enable watchdog reset
    ESP.wdtEnable(1);

    if (result == node.ku8MBSuccess)
    {
        for (j = 0; j < dataReadLength; j++)
            data[j] = (node.getResponseBuffer(j));

        Serial.println("data[0]: " + String(data[0], HEX) + " " + String(data[0]));
        Serial.println("data[1]: " + String(data[1], HEX) + " " + String(data[1]));

        int value = hexToDec(String(data[0], HEX));
        Serial.println("\nvalue = " + String(value));
        humidity = (float)value / 10;
        value = hexToDec(String(data[1], HEX));
        Serial.println("value = " + String(value));
        temperature = (float)value / 10;
    }
    else
    {
        Serial.print("Connect modbus fail. REG >>> ");
        Serial.println(Reg_Address); // Debug
        delay(1000);
    }
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