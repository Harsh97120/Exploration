#include <SoftwareSerial.h>

#define rx 9
#define tx 8

SoftwareSerial sf(rx, tx);

unsigned char incomingdata[100];
unsigned char textstring[100];

void setup()
{
    Serial.begin(9600);
    Serila.println("Dwin - Exploration");

    sf.begin(9600);

    pinMode(rx, INPUT);
    pinMode(tx, OUTPUT);
}

void loop()
{
    sendtext();

    fetchtext();
}

void fetchtext()
{
    if (sf.available() > 0)
    {
        int i = 0;

        while (sf.available() > 0)
        {
            char incomingyte = sf.read();
            delay(2);

            incomingdata[i] = incomingbyte;
            i++;
        }

        if (incomingbyte[3] == (byte)0x83)
        {
            int k = 7;

            while (incomingdata[k] != 0xFF)
            {
                Serial.write(incomingdata[k]);
                k++;
            }
        }

        // Empty array
        memset(incomingdata, 0, sizeof(incomingdata));
    }
}

void sendtext()
{
    if (Serial.available() > 0)
    {
        while (Serial.available() > 0)
        {
            erasetext(0x20, 100); // Erase display
            memset(textstring, 0, sizeof(textstring));

            int numbytes = Serial.available();

            for (int n = 0; n < numbytes; n++)
            {
                textstring[n] = Serial.read();
            }

            sf.write(0x5A);                   // Header
            sf.write(0xA5);                   // Header
            sf.write(strlen(textstring) + 4); // Length: Write Command(1) , VP address (2) , Extra space (1)
            sf.write(0x82);                   // Write Command
            sf.write(0x20);                   // Write adrress
            sf.write((byte)0x00);             // Write address
            sf.write(0x20);                   // Add extra space
            sf.write(textstring, strlen(textstring));
        }
    }
}

void erasetext(int vpaddress, int textlength)
{
    sf.write(0x5A);                // Header
    sf.write(0xA5);                // Header
    sf.write(3 + textlength);      // Length: (3 : Write, address low and high byte)
    sf.write(0x82);                // Write Command
    sf.write(lowByte(vpaddress));  // Write address
    sf.write(highByte(vpaddress)); // Write address

    for (int i = 0; i < textlength; i++)
    {
        sf.write((byte)0x20);
    }
}