/*
Using serial communication to write a holding register inside the slave

RX and TX pins of one controller is connected to the opposite pin of the other controller, i.e,
RX (Master) -> TX (Slave)
TX (Master) -> RX (Slave)

Refer to comments in the code for more details
*/

#include <ModbusRTU.h>

#define RX 13
#define TX 12
#define HREG 1
#define SLAVE_ID 1

ModbusRTU mb;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX, TX);
  mb.begin(&Serial2);  //Initialize Modbus on Serial2 Channel
  mb.setBaudrate(9600);
  mb.slave(SLAVE_ID);   //Tells the current device it is a slave with Id = SLAVE_ID
  mb.addHreg(HREG, 0);  //Assigns Register HREG to be a holding register with the inital value 0
}

float read_value = 0;

void loop() {
  mb.task();

  read_value = mb.Hreg(HREG); //Reads the value of HREG and stores it in read_value
  Serial.print("Reading the value of holding register : ");
  Serial.println(read_value);

  delay(1000);
}