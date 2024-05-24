#include <SoftwareSerial.h>

byte TX_PIN  = 4, RX_PIN = 5;
byte DE_RE_PIN = 16 , LED_PIN = 2;

SoftwareSerial Soft_Serial(RX_PIN,TX_PIN);

long counter = 0;

void setup()
{
    Serial.begin(115200);
    Soft_Serial.begin(9600);

    pinMode(DE_RE_PIN, OUTPUT);
    digitalWrite(DE_RE_PIN , HIGH);
    pinMode(LED_PIN , OUTPUT);
    digitalWrite(LED_PIn , HIGH);

    Serial.println("Welcome....\n");
    Serial.println("Sender Start\n");

}

void loop()
{
    counter++;

    Serial.println("Counter = " + String(counter));
    Soft_Serial.write(counter);
    digitalWrite(LED_PIN,LOW);
    delay(100);
    digitalWrite(LED_PIN , HIGH);
    delay(1000);
}