#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#else
#include <WiFi.h>          //https://github.com/esp8266/Arduino
#endif

//needed for library
#include <DNSServer.h>
#if defined(ESP8266)
#include <ESP8266WebServer.h>
#else
#include <WebServer.h>
#endif
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

#define LED_BUILTIN 2

void setup()
{
    Serial.begin(115200);
    Serial.println("Hello");
    delay(200);
    pinMode(LED_BUILTIN, OUTPUT);
      //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();

    
    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH); // turn on the LED
    delay(500);                      // wait for half a second or 500 milliseconds
    digitalWrite(LED_BUILTIN, LOW);  // turn off the LED
    delay(500);                      // wait for half a second or 500 milliseconds
    Serial.println("LOOP");
}