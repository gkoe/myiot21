#include "BleMiflora.h"

#include <WiFi.h>
#include <M5Stack.h>

#include "MiFloraConfig.h"

const char* mqtt_server = "10.0.0.125";

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001204-0000-1000-8000-00805f9b34fb");

// The characteristic of the remote service we are interested in.
static BLEUUID uuid_version_battery("00001a02-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_sensor_data("00001a01-0000-1000-8000-00805f9b34fb");
static BLEUUID uuid_write_mode("00001a00-0000-1000-8000-00805f9b34fb");

static BLERemoteCharacteristic* pRemoteCharacteristic;

void BleMifloraClass::init(std::string macAddress, const char* name){
    BLEDevice::init("");
    _name = name;
    WiFiClient* wifiClient = new WiFiClient();
    _pubSubClient = new PubSubClient(*wifiClient);
    _pubSubClient->setServer(mqtt_server, 1883);
    while (!_pubSubClient->connected()) {
        Serial.print(F("!BLE: Attempting MQTT connection..."));
        // Attempt to connect
        if (_pubSubClient->connect("floraClient")) {
            Serial.println(F("*BLE: connected"));
        } 
        else {
        Serial.print(F("!BLE: failed, rc="));
        Serial.print(_pubSubClient->state());
        Serial.println(F("*BLE: try again in 5 seconds"));
        // Wait 5 seconds before retrying
        delay(5000);
        }
    }
    _bleAddress = new BLEAddress(macAddress);
}

void BleMifloraClass::publishToMqtt(const char* sensorType, int value ){
  char payload[64];
  char topic[100];

  snprintf(topic, 100, "plant/%s/%s", _name, sensorType);
  Serial.print(F("!BLE: Topic: "));
  Serial.println(topic);
  snprintf(payload, 64, "%d", value);
  Serial.print("Payload: ");
  Serial.println(payload);
  _pubSubClient->publish(topic, payload);
  M5.Lcd.printf("%s: %s", topic, payload);
  // DisplayLogger.print(topic);
  // DisplayLogger.print(":");
  // DisplayLogger.println(payload);

}

bool BleMifloraClass::getSensorDataAndPublishToMqtt() {
  Serial.print((F("*BLE: create connection to Flora device at ")));
  Serial.println(_bleAddress->toString().c_str());
  BLEClient*  bleClient  = BLEDevice::createClient();
  // Connect to the remove BLE Server.
  if (!bleClient->connect(*_bleAddress)) {
    M5.Lcd.printf("NO CONN: %s", _name);
      // DisplayLogger.print("NO CONN: ");
      // DisplayLogger.println(name);
      Serial.print(F("!BLE: No connection to: "));
      Serial.println(_name);
      return false;
  }
  Serial.print(F("!BLE: Connected with: "));
  Serial.println(_name);
  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = bleClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print(F("!BLE: Failed to find our service UUID: "));
    Serial.println(serviceUUID.toString().c_str());
    return false;
  }
  Serial.println(F("*BLE: Found miflora service"));
  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_write_mode);
  uint8_t buf[2] = {0xA0, 0x1F};  // Zuerst auf eine Charakteristik schreiben
  pRemoteCharacteristic->writeValue(buf, 2, true);
  delay(500);
  // Sensordaten über Charakteristik auslesen
  pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_sensor_data);
  Serial.println(pRemoteService->toString().c_str());
  if (pRemoteCharacteristic == nullptr) {
    Serial.print(F("!BLE: Failed to find our characteristic UUID: "));
    Serial.println(uuid_sensor_data.toString().c_str());
    return false;
  }
  Serial.println(F("*BLE: found miflora characteristic"));
  // Read the value of the characteristic.
  std::string value = pRemoteCharacteristic->readValue();
  Serial.print(F("*BLE: The characteristic value was: "));
  const char *val = value.c_str();

  Serial.print("Hex: ");
  for (int i = 0; i < 16; i++) {
    Serial.print((int)val[i], HEX);
    Serial.print(" ");
  }
  Serial.println(" ");
  // calculate measurements
  float temp = (val[0] + val[1] * 256) / ((float)10.0);
  int moisture = val[7];
  int light = val[3] + val[4] * 256;
  int conductivity = val[8] + val[9] * 256;
  
//   char payload[64];
//   char topic[100];
  
  // Serial.print("Temperature: ");
  // Serial.println(temp);
  // sprintf(topic, "flora/%s/temperature",name);
  // snprintf(buffer, 64, "%f", temp);
  // client.publish(topic, buffer);

  publishToMqtt("moisture", moisture);
  

//   snprintf(topic, 100, "plant/%s/moisture", name);
//   Serial.print("Topic: ");
//   Serial.println(topic);
//   snprintf(payload, 64, "%d", moisture);
//   Serial.print("Payload: ");
//   Serial.println(payload);
//   client.publish(topic, payload);
//   M5.Lcd.printf("%s: %s", topic, payload);
//   // DisplayLogger.print(topic);
//   // DisplayLogger.print(":");
//   // DisplayLogger.println(payload);

  delay(500);

  publishToMqtt("light", light);


//   snprintf(topic, 100, "plant/%s/light", name);
//   Serial.print("Topic: ");
//   Serial.println(topic);
//   snprintf(payload, 64, "%d", light);
//   Serial.print("Payload: ");
//   Serial.println(payload);
//   client.publish(topic, payload);

  delay(500);

  // Serial.print("Conductivity: ");
  // Serial.println(conductivity);
  // snprintf(buffer, 64, "%d", conductivity);
  // client.publish(MQTT_CONDUCTIVITY, buffer);

    Serial.println(F("*BLE: Trying to retrieve battery level..."));
    pRemoteCharacteristic = pRemoteService->getCharacteristic(uuid_version_battery);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print(F("!BLE: failed to find our characteristic UUID: "));
      Serial.println(uuid_sensor_data.toString().c_str()); 
      return false;
    }
    Serial.println(F("*BLE: found battery characteristic"));
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

    publishToMqtt("battery", battery);


    // snprintf(topic, 100, "plant/%s/battery", name);
    // Serial.print("Topic: ");
    // Serial.println(topic);
    // snprintf(payload, 64, "%d", battery);
    // Serial.print("Payload: ");
    // Serial.println(payload);
    // client.publish(topic, payload);

    bleClient->disconnect();
}

BleMifloraClass BleMiflora;


