#pragma once

#include "nvs.h"
#include <Constants.h>

class EspConfigClass
{

public:
	void init();
	void clearConfig();
	void getConfig(char* buffer, int size);
	char* getNvsStringValue(const char* key);
	int getNvsIntValue(const char *key);
	void setNvsStringValue(const char* key, const char* value);
	void setNvsIntValue(const char *key, int value);
	void deleteKey(const char* key);
	char* getSsid();
	char* getPassword();
	char* getThingName();
	char* getMqttBroker();
	int getMqttBrokerPort();


private:
	char* _ssid;
	char* _password;
	char* _mqttBroker;
	char* _thingName;
	char* _mqttBrokerPort;

	nvs_handle _nvsHandle;


};

extern EspConfigClass EspConfig;


