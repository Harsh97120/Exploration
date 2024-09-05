#include <ModbusRTU.h>

#define RX 13
#define TX 12

#define SLAVE_ID 1
#define HREG 1

bool readTask = false;

ModbusRTU mb;

uint16_t value;

void setup()
{
    Serial.begin(9600, SERIAL_8N1);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializes Modbus on Serial2
    mb.master();
}

void loop()
{

    // mb.readIreg(SLAVE_ID, 1, value, 1, cbRead);
    // if(readTask)
    // {
    //     Serial.print("Value is Received.\n");
    //     Serial.print("Received value is: ");
    //     Serial.print(value);
    // }

    // read_value = mb.Hreg(HREG); // Reads the value of HREG and stores it in read_value
    // Serial.print("Reading the value of holding register : ");
    // Serial.println(read_value);

    if (!mb.readHreg(SLAVE_ID, HREG, &value, 1, cb))
    {
        Serial.println("Modbus request failed");
    }

    mb.task();
    delay(1000);
}

bool cbRead(Modbus::ResultCode event, uint16_t transactionId, void *data)
{
    if (event == Modbus::EX_SUCCESS)
    {
        Serial.println("Read successful");
        readTask = true;
        return true;
    }
    else
    {
        Serial.print("Read failed with code: ");
        Serial.println(event);
        return false;
    }
}

bool cb(Modbus::ResultCode event, uint16_t transactionId, void *data)
{
    Serial.print("Request result: 0x");
    Serial.println(event, HEX);
    if (event == Modbus::EX_SUCCESS)
    {
        Serial.print("Received value: ");
        Serial.println(value);
    }
    return true;
}