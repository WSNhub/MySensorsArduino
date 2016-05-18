/*
 Based in SDM630 from Bart Hillaert 
 */
#include <SoftwareSerial.h>
#include <SPI.h>

//Std serial communication
#define tx 1
#define rx 0
#define SlaveAddress 0x01   //see NodeId in SDM220 setting : default = 1

#define RS485_TXenable 4    //DE & RE
#define RS485_RX  2        //RO
#define RS485_TX  3        //DI
#define LOOPTIME  10000

#define Function 0x04

byte TXbuf[20];
byte RXbuf[25];
byte dummy[25];

SoftwareSerial mySerial(RS485_RX, RS485_TX); 

void setup()  
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
  mySerial.begin(9600);
  
  pinMode(RS485_TXenable,OUTPUT);
  digitalWrite(RS485_TXenable,0);

  Serial.println("Started");
}


unsigned int CRC16_2(unsigned char *buf, int len)
{  
  unsigned int crc = 0xFFFF;
  for (int pos = 0; pos < len; pos++)
  {
  crc ^= (unsigned int)buf[pos];    // XOR byte into least sig. byte of crc

  for (int i = 8; i != 0; i--) {    // Loop over each bit
    if ((crc & 0x0001) != 0) {      // If the LSB is set
      crc >>= 1;                    // Shift right and XOR 0xA001
      crc ^= 0xA001;
    }
    else                            // Else LSB is not set
      crc >>= 1;                    // Just shift right
    }
  }
  return crc;
}


void TXreadInputRegister(unsigned int regNr){
  char c;
  unsigned int crc;
  unsigned int startAddress = (regNr-1);
  
  digitalWrite(RS485_TXenable,1);
  delay(1);
  mySerial.flush();
  
  //header
  TXbuf[0]=SlaveAddress;
  TXbuf[1]=Function;
  //Starting Address
  
  Serial.print("Address ");Serial.println(startAddress);
  TXbuf[2]=(startAddress&0xFF00)>>8; //high byte
  TXbuf[3]=(startAddress&0x00FF);    //low byte
  //Number of points 2
  TXbuf[4]=0x00;
  TXbuf[5]=0x02;
  //CRC
  crc=CRC16_2(TXbuf,6);
  TXbuf[6]=(crc & 0x00FF);
  TXbuf[7]=(crc & 0xFF00)>>8;
  
  //SEND      
  for (int x=0;x<8;x++){
    mySerial.print(char(TXbuf[x]));
    delay(1);
  }
  digitalWrite(RS485_TXenable,0);
  delay(1);
}


float extractData(){
  union {
    byte asBytes[4];
    float asFloat;
  } foo;
  foo.asBytes[0] = RXbuf[6];
  foo.asBytes[1] = RXbuf[5];
  foo.asBytes[2] = RXbuf[4];
  foo.asBytes[3] = RXbuf[3];
  return foo.asFloat;
}

int receiveQuery(){
  byte x,i;
  byte valid;
  unsigned int crc;
  delay(10);
  while (not mySerial.available()){/*Serial.print(".");*/}
  //Serial.print("RECEIVED ");
  x=0;
  byte tmp;
  while (mySerial.available()) 
    {
      if (x==0) { //skip 0x00 for the first 0x00 bytes
        tmp = mySerial.read();
        if (tmp > 0x0){
          RXbuf[x] = tmp;
          x++;
        }
      }
      else { 
        RXbuf[x] = mySerial.read();
        x++;
      }
      delay(10);
    }
  
  //copy to dummy array to calculate CRC, only 9 bytes needed in RX packet
  for (int x=0; (x < 9);x++){
    dummy[x]=RXbuf[x];
  }
  //Serial.println("New dummy array: ");
  //for (int i=0;i<9;i++){ Serial.print(dummy[i],HEX); Serial.print(" ");}
  
  crc=CRC16_2(dummy,7);
  if (RXbuf[7]==(crc & 0x00FF)) {
    if (RXbuf[8]==(crc & 0xFF00)>>8){
      Serial.print("\t\t[CRC ok] ");
      valid=1;
    }
    else {
      Serial.print("[CRC ERROR H] -----------------------");
      Serial.print(RXbuf[8],HEX); Serial.print(" <> ");Serial.println((crc & 0xFF00)>>8,HEX);
      valid=0;
    }
  }
  else{
      Serial.print("[CRC ERROR L] -----------------------");
      Serial.print(RXbuf[7],HEX); Serial.print(" <> ");Serial.println(crc & 0x00FF,HEX);
      valid=0;
  }
  
}

float x1,x2,x3,pt,pinport,pexport=0.0;
char buf[10];
    
void loop() // run over and over
{ 
    Serial.print("Line to neutral (V): "); TXreadInputRegister(1); receiveQuery(); pt = extractData(); Serial.println(pt);      
    Serial.print("Current         (A): "); TXreadInputRegister(7);  receiveQuery(); pt = extractData();Serial.println(pt);
    Serial.print("Active power    (W): "); TXreadInputRegister(13); receiveQuery(); {pt = extractData();Serial.println(pt);}
    Serial.print("Apparent power  (VoltAmps): "); TXreadInputRegister(19); receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Reactive power  (VAr): "); TXreadInputRegister(25);  receiveQuery(); {pt = extractData();Serial.println(pt);}
    Serial.print("Power factor  (None): "); TXreadInputRegister(31); receiveQuery(); {pt = extractData();Serial.println(pt);}    
    Serial.print("Phase angle (Degree): "); TXreadInputRegister(37);  receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Frequency  (Hz): "); TXreadInputRegister(71);  receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Import active energy  (kwh): "); TXreadInputRegister(73);  receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Export active energy  (kwh): "); TXreadInputRegister(75);  receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Import reactive energy  (kvarh): "); TXreadInputRegister(77);  receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("Export reactive energy  (kvarh): "); TXreadInputRegister(79); receiveQuery();  {pt = extractData();Serial.println(pt);}
    Serial.print("total active energy  (kwh): "); TXreadInputRegister(343);  receiveQuery(); {pt = extractData();Serial.println(pt);}
    Serial.print("total reactive energy  (kvarh): "); TXreadInputRegister(345); receiveQuery(); {pt = extractData();Serial.println(pt);}    
    Serial.println("__Waiting__________________________________________________");
    delay(LOOPTIME);

}

