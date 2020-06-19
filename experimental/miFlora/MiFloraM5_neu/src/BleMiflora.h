#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include "BLEDevice.h"

class BleMifloraClass
{
 public:
	void init(std::string macAddress, const char* name);
    bool getSensorDataAndPublishToMqtt();
 private:
    PubSubClient* _pubSubClient;
    BLEAddress*   _bleAddress;
    const char* _name;
    void publishToMqtt(const char* sensorType, int value );
};

extern BleMifloraClass BleMiflora;

