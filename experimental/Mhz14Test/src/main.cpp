#include <Arduino.h>

#define RXD2 16  // am MHZ verbunden mit Pin 19
#define TXD2 17  // am MHZ verbunden mit Pin 18

HardwareSerial serialMhz(2);

uint8_t cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79};
uint8_t reset[9] = {0xFF,0x01,0x87,0x00,0x00,0x00,0x00,0x00,0x78};

void setup() {
  Serial.begin(115200);
  serialMhz.begin(9600, SERIAL_8N1, RXD2, TXD2);
  serialMhz.write(reset,9);
  delay(100);

}

void loop() {
  uint8_t response[9] = {};
  uint8_t index = 0;
  bool responseReceived = false;
  serialMhz.write(cmd,9);
  while(serialMhz.available()>0)
  {
    response[index++]=serialMhz.read();
    responseReceived=true;
  }
  if(responseReceived)
  {
    if(index==9){
      uint16_t co2=0;
      co2 += (uint16_t)response[2] <<8;
      co2 += response[3];
      Serial.printf("Co2-Gehalt: %d\n",co2);
    }
    else{
      Serial.printf("!Statt 9 Zeichen nur %d Zeichen empfangen\n",index);      
    }
    responseReceived=false;
  }
  else{
    Serial.printf("!Kein Empfang von CO2-Sensor\n");      
  }
  delay(1000);
}
