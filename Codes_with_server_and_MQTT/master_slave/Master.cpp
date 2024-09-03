#include <SoftwareSerial.h>
#include <ModbusMaster.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Wifi details
const char *ssid = "Your SSID";
const char *password = "Your Password";

// MQTT Broker Details
String device_id = "Device0001";
const char *mqtt_server = "mqtt://localhost:1883";
const int mqtt_port = 1883;
const char *mqtt_user = "Device0001";
const char *mqtt_password = "Device0001";
const char *mqtt_clientId = "Deivce_Device0001";
const char *topic_publish = "G6_SensorData";
const char *topic_subscribe = "G6_CommandRequest";

WiFiClient esp_client;
void callback(char *topic, byte *payload, unsigned int length);
PubSubClient mqtt_client(mqtt_server, mqtt_port, callback, esp_client);

#define RX 4
#define TX 5
#define DE_RE 16

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

    setup_wifi();
    mqtt_connect();
    Serial.println("\n\nWelcome to IoT");
    Serial.println("Master Start.\n");
}

void loop()
{
    Read_Data();
    Print_Data();
    save_data();

    delay(5000);
}

void Read_Data()
{
    byte i = 0, index_for_val = 0;
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

void save_data()
{
    String pkt = "{";
    pkt += "\"device_id\": \"" + device_id + "\", ";
    pkt += "\"type\": \"Temperature\", ";
    pkt += "\"value\": " + String(values[0]) + "";
    pkt += "}";
    mqtt_publish((char *)pkt.c_str());

    String pkt2 = "{";
    pkt2 += "\"device_id\": \"" + device_id + "\", ";
    pkt2 += "\"type\": \"Humidity\", ";
    pkt2 += "\"value\": " + String(values[1]) + "";
    pkt2 += "}";
    mqtt_publish((char *)pkt2.c_str());

    String pkt3 = "{";
    pkt3 += "\"device_id\": \"" + device_id + "\", ";
    pkt3 += "\"type\": \"Pressure\", ";
    pkt3 += "\"value\": " + String(values[2]) + "";
    pkt3 += "}";
    mqtt_publish((char *)pkt3.c_str());

    String pkt4 = "{";
    pkt4 += "\"device_id\": \"" + device_id + "\", ";
    pkt4 += "\"type\": \"voltage\", ";
    pkt4 += "\"value\": " + String(values[3]) + "";
    pkt4 += "}";
    mqtt_publish((char *)pkt4.c_str());

    if (!mqtt_client.loop())
    {
        mqtt_connect();
    }

    delay(1000);
}

void setup_wifi()
{
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println("\"" + String(ssid) + "\"");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void mqtt_connect()
{
    while (!mqtt_client.connected())
    {
        Serial.println("Attempting MQTT connection...");

        if (mqtt_client.connect(mqtt_clientId))
        {
            Serial.println("MQTT Client Connected");
            mqtt_subscribe(topic_subscribe);
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt_client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void mqtt_publish(char *data)
{
    mqtt_connect();
    Serial.println("Publish Topic: \"" + String(topic_publish) + "\"");
    if (mqtt_client.publish(topic_publish, data))
    {
        Serial.println("Publish \"" + String(data) + "\" ok");
    }
    else
    {
        Serial.println("Publish \"" + String(data) + "\" failed");
    }
}

void mqtt_subscribe(const char *topic)
{
    if (mqtt_client.subscribe(topic))
    {
        Serial.println("Subscribe \"" + String(topic) + "\" ok");
    }
    else
    {
        Serial.println("Subscribe \"" + String(topic) + "\" failed");
    }
}

void callback(char *topic, byte *payload, unsigned int length)
{
    String command;
    Serial.print("\n\nMessage arrived [");
    Serial.print(topic);
    Serial.println("] ");
    for (int i = 0; i < length; i++)
        command += (char)payload[i];

    if (command.length() > 0)
        Serial.println("Command receive is : " + command);

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, command);
    JsonObject obj = doc.as<JsonObject>();

    String id = obj[String("device_id")];
    String type = obj[String("type")];
    String value = obj[String("value")];
    Serial.println("\nCommand device_id is : " + id);
    Serial.println("Command type is : " + type);
    Serial.println("Command value is : " + value);
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
