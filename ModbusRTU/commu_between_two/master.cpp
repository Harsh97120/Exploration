#include <ModbusRTU.h>

#define RX 13
#define TX 12

#define SLAVE_ID 1

bool readTask = false;

ModbusRTU mb;

uint16_t value;

void setup()
{
    Serial.begin(9600, SERIAL_8N1);
    Serial2.begin(9600, SERIAL_8N1, RX, TX);
    mb.begin(&Serial2); // Initializes Modbus on Serial2
    mb.slave(1);

    // Set up holding register
    mb.addHreg(1, 0);
}

void loop()
{
    mb.task();
    // mb.readIreg(SLAVE_ID, 1, value, 1, cbRead);

    // if(readTask)
    // {
    //     Serial.print("Value is Received.\n");
    //     Serial.print("Received value is: ");
    //     Serial.print(value);
    // }

    read_value = mb.Hreg(HREG); // Reads the value of HREG and stores it in read_value
    Serial.print("Reading the value of holding register : ");
    Serial.println(read_value);

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