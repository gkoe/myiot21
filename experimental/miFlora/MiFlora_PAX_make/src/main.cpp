#include <Arduino.h>
//#include "globals.h"
#include "wifiscan.h"

void setup()
{
  Serial.begin(115200);                 //Initialisierung der seriellen Schnittstelle
	Serial.println();
	Serial.println();
	Serial.println(F("Wifi-Sniffer"));

  wifi_sniffer_init();
}

void loop()
{
 yield();
}