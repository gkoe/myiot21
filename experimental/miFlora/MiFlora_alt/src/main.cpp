/**
   A BLE client for the Xiaomi Mi Plant Sensor, pushing measurements to an MQTT server.
   
   MIT License

   Copyright (c) 2017 Sven Henkel

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#include "BLEDevice.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <DisplayLogger.h>
#include <rom/rtc.h>

// https://github.com/espressif/arduino-esp32/blob/master/libraries/ESP32/examples/ResetReason/ResetReason.ino

#include "config.h"

#define MI_FLORAS 5


std::string macAddresses[MI_FLORAS] = { "C4:7C:8D:66:01:E6", "C4:7C:8D:65:58:87", "C4:7C:8D:64:3B:61", "C4:7C:8D:65:C0:7B", "C4:7C:8D:65:B5:D5" };
char *names[MI_FLORAS] = {"agave", "oleander","bougainvillea", "strelizie", "sagopalme"};


RTC_DATA_ATTR int bootCount = 1;
RTC_DATA_ATTR long publishTimes[] = {0,0};

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");

// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLEAddress floraAddress(FLORA_ADDR);

static int doConnect = 0;
static boolean connected = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;

WiFiClient espClient;
PubSubClient client(espClient);


bool getSensorData(BLEAddress pAddress, char *name, bool getBattery) {
  Serial.print("Forming a connection to Flora device at ");
  Serial.println(pAddress.toString().c_str());

  BLEClient*  pClient  = BLEDevice::createClient();

  // Connect to the remove BLE Server.
  if (!pClient->connect(pAddress)) {
      DisplayLogger.print("NO CONN: ");
      DisplayLogger.println(name);
      Serial.print("!!! No connection to: ");
      Serial.println(name);
      return false;
  }
  Serial.print("Connected with: ");
  Serial.println(name);

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(" - Found our service");

  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
  uint8_t buf[2] = {0xA0, 0x1F};
  pRemoteCharacteristic->writeValue(buf, 2, true);

  delay(500);

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
  Serial.println(pRemoteService->toString().c_str());
  if (pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(uuid_sensor_data.toString().c_str());
    return false;
  }
  Serial.println(" - Found our characteristic");


  // Read the value of the characteristic.
  std::string value = pRemoteCharacteristic->readValue();
  Serial.print("The characteristic value was: ");
  const char *val = value.c_str();

  Serial.print("Hex: ");
  for (int i = 0; i < 16; i++) {
    Serial.print((int)val[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");

  float temp = (val[0] + val[1] * 256) / ((float)10.0);
  int moisture = val[7];
  int light = val[3] + val[4] * 256;
  int conductivity = val[8] + val[9] * 256;
  
  char payload[64];
  char topic[100];
  
  // Serial.print("Temperature: ");
  // Serial.println(temp);
  // sprintf(topic, "flora/%s/temperature",name);
  // snprintf(buffer, 64, "%f", temp);
  // client.publish(topic, buffer);
  

  snprintf(topic, 100, "plant/%s/moisture", name);
  Serial.print("Topic: ");
  Serial.println(topic);
  snprintf(payload, 64, "%d", moisture);
  Serial.print("Payload: ");
  Serial.println(payload);
  client.publish(topic, payload);
  DisplayLogger.print(topic);
  DisplayLogger.print(":");
  DisplayLogger.println(payload);
  delay(500);


  snprintf(topic, 100, "plant/%s/light", name);
  Serial.print("Topic: ");
  Serial.println(topic);
  snprintf(payload, 64, "%d", light);
  Serial.print("Payload: ");
  Serial.println(payload);
  client.publish(topic, payload);

  // Serial.print("Conductivity: ");
  // Serial.println(conductivity);
  // snprintf(buffer, 64, "%d", conductivity);
  // client.publish(MQTT_CONDUCTIVITY, buffer);

//  if (getBattery) {
    Serial.println("Trying to retrieve battery level...");
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(uuid_sensor_data.toString().c_str()); 
      return false;
    }
    Serial.println(" - Found our characteristic");

    // Read the value of the characteristic...
    value = pRemoteCharacteristic->readValue();
    Serial.print("The characteristic value was: ");
    const char *val2 = value.c_str();
    Serial.print("Hex: ");
    for (int i = 0; i < 16; i++) {
      Serial.print((int)val2[i], HEX);
      Serial.print(" ");
    }
    Serial.println(" ");

    int battery = val2[0];
    Serial.print("Battery: ");
    snprintf(topic, 100, "plant/%s/battery", name);
    Serial.print("Topic: ");
    Serial.println(topic);
    snprintf(payload, 64, "%d", battery);
    Serial.print("Payload: ");
    Serial.println(payload);
    client.publish(topic, payload);
//  }

  pClient->disconnect();
}

void taskDeepSleep( void * parameter )
{
  delay(SLEEP_WAIT * 1000);
  esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ll);
  Serial.println("Going to sleep now.");
  esp_deep_sleep_start();
}

void verbose_print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : Serial.println ("Vbat power on reset");break;
    case 3  : Serial.println ("Software reset digital core");break;
    case 4  : Serial.println ("Legacy watch dog reset digital core");break;
    case 5  : Serial.println ("Deep Sleep reset digital core");break;
    case 6  : Serial.println ("Reset by SLC module, reset digital core");break;
    case 7  : Serial.println ("Timer Group0 Watch dog reset digital core");break;
    case 8  : Serial.println ("Timer Group1 Watch dog reset digital core");break;
    case 9  : Serial.println ("RTC Watch dog Reset digital core");break;
    case 10 : Serial.println ("Instrusion tested to reset CPU");break;
    case 11 : Serial.println ("Time Group reset CPU");break;
    case 12 : Serial.println ("Software reset CPU");break;
    case 13 : Serial.println ("RTC Watch dog Reset CPU");break;
    case 14 : Serial.println ("for APP CPU, reseted by PRO CPU");break;
    case 15 : Serial.println ("Reset when the vdd voltage is not stable");break;
    case 16 : Serial.println ("RTC Watch dog reset digital core and rtc module");break;
    default : Serial.println ("NO_MEAN");
  }
}


void setup() {
  Serial.begin(115200);
  if(rtc_get_reset_reason(0) != 5){
    DisplayLogger.init(ANIMATION_NONE, true, TO_LONG_BEHAVIOUR_NEWLINE);
  }
  verbose_print_reset_reason(rtc_get_reset_reason(0));

   delay(1000);
  
  xTaskCreate(      taskDeepSleep,          /* Task function. */
                    "TaskDeepSleep",        /* String with name of task. */
                    10000,                  /* Stack size in words. */
                    NULL,                   /* Parameter passed as input of the task */
                    1,                      /* Priority of the task. */
                    NULL);                  /* Task handle. */
  
  
  Serial.println("Starting Flora client...");
  BLEDevice::init("");

  Serial.println("Connecting WiFi...");
  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");

  client.setServer(mqtt_server, 1883);
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("floraClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }

    Serial.print("BootCount: ");
    Serial.println(bootCount);
    std::string macAddr = macAddresses[bootCount];
    char *name = names[bootCount];
    Serial.print("Read from ");
    Serial.println(name);
    BLEAddress* floraAddress = new BLEAddress(macAddr);
    if(getSensorData(*floraAddress, name, ((bootCount % BATTERY_INTERVAL) == 0))){
        Serial.println("Sensordaten übertragen ");
        //Serial.println(name);
    }
    bootCount++;
    if(bootCount >= MI_FLORAS){
      bootCount=0;
    }


    // for(uint8_t i=0; i<MI_FLORAS; i++){
    //     std::string macAddr = macAddresses[i];
    //     std::string name = names[i];
    //     BLEAddress* floraAddress = new BLEAddress(macAddr);
    //     if(getSensorData(*floraAddress, name, ((bootCount % BATTERY_INTERVAL) == 0))){
    //         Serial.println("Sensordaten übertragen ");
    //         //Serial.println(name);
    //     }
    //   bootCount++;
    // }


  }

  delay(1000);
  Serial.println("End of setup");

} // End of setup.


// This is the Arduino main loop function.
void loop() {
//   getSensorData(floraAddress, ((bootCount % BATTERY_INTERVAL) == 0));
//   Serial.println("End of transmission");
  delay(5000);
} // End of loop