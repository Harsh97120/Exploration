#include <ModbusRTU.h>

#define RX 13
#define TX 12
#define SLAVE_ID 1

#define HREG1 1
#define HREG2 2
#define HREG3 3
#define HREG4 4
#define HREG5 5
#define HREG6 6
#define HREG7 7
#define HREG8 8
#define HREG9 9
#define HREG10 10
#define HREG11 11
#define HREG12 12

ModbusRTU mb;


void setup() {
  
  Serial.begin(9600, SERIAL_8N1);
  Serial2.begin(9600, SERIAL_8N1, RX, TX);
  mb.begin(&Serial2); 
  mb.setBaudrate(9600);
  mb.slave(SLAVE_ID);


  mb.addHreg(HREG1, 0);
  mb.addHreg(HREG2, 0); 
  mb.addHreg(HREG3, 0);
  mb.addHreg(HREG4, 0);
  mb.addHreg(HREG5, 0);
  mb.addHreg(HREG6, 0);
  mb.addHreg(HREG7, 0);
  mb.addHreg(HREG8, 0);
  mb.addHreg(HREG9, 0);
  mb.addHreg(HREG10, 0);
  mb.addHreg(HREG11, 0);
  mb.addHreg(HREG12, 0);
}

int read_value1 = 0 ;
int read_value2 = 0 ;
int read_value3 = 0 ;
int read_value4 = 0 ;
int read_value5 = 0 ;
int read_value6 = 0 ;
int read_value7 = 0 ;
int read_value8 = 0 ;
int read_value9 = 0 ;
int read_value10 = 0 ;
int read_value11 = 0 ;
int read_value12 = 0 ;

float temperature = 0 ;
float humidity = 0 ;
float pressure = 0 ;
float ppm = 0 ;
float current = 0 ;
float voltage = 0 ;

void loop() {
  mb.task();
  yield();
  delay(100);



  read_value1 = mb.Hreg(HREG1);
  read_value2 = mb.Hreg(HREG2);
  read_value3 = mb.Hreg(HREG3);
  read_value4 = mb.Hreg(HREG4); 
  read_value5 = mb.Hreg(HREG5);
  read_value6 = mb.Hreg(HREG6);
  read_value7 = mb.Hreg(HREG7);
  read_value8 = mb.Hreg(HREG8);
  read_value9 = mb.Hreg(HREG9);
  read_value10 = mb.Hreg(HREG10);
  read_value11 = mb.Hreg(HREG11);
  read_value12 = mb.Hreg(HREG12);
  // For temperature readings
  Serial.print("Reading the value of holding register1 and 2 (Temp) : ");
  temperature=read_value1+(1.0*(read_value2))/100;
  Serial.println(temperature);

  delay(100);
// For humidity readings
  Serial.print("Reading the value of holding register3 and 4 (humi) : ");
  humidity=read_value3+(1.0*(read_value4))/100;
  Serial.println(humidity);
  delay(100);

  Serial.print("Reading the value of holding register5 and 6 (pressure) : ");
  pressure=read_value5+(1.0*(read_value6))/100;
  Serial.println(pressure);
  delay(100);

  Serial.print("Reading the value of holding register7 and 8 (ppm) : ");
  ppm=read_value7+(1.0*(read_value8))/100;
  Serial.println(ppm);
  delay(100);

  Serial.print("Reading the value of holding register9 and 10 (current) : ");
  current=read_value9+(1.0*(read_value10))/100;
  Serial.println(current);
  delay(100);

  Serial.print("Reading the value of holding register11 and 12 (voltage) : ");
  voltage=read_value11+(1.0*(read_value12))/100;
  Serial.println(voltage);
  




  

  // updateLedState();
  // printValues(); //printing just to confirm that registers are updated
  delay(100);
}
