#include <RgbLed.h>
#include <Thing.h>
#include <Logger.h>

RgbLed::RgbLed(const int pinR, const int pinG, const int pinB, const char* thingName, 
		const char* name) : IotActor(thingName, name)
{
	int freq = 500;
	int resolution = 8;
	_pinR = pinR;
	_pinG = pinG;
	_pinB = pinB;
	pinMode(_pinR, OUTPUT);
	pinMode(_pinG, OUTPUT);
	pinMode(_pinB, OUTPUT);
	ledcSetup(_ledChannelR, freq, resolution);
	ledcSetup(_ledChannelG, freq, resolution);
	ledcSetup(_ledChannelB, freq, resolution);
	ledcAttachPin(_pinR, _ledChannelR);
	ledcAttachPin(_pinG, _ledChannelG);
	ledcAttachPin(_pinB, _ledChannelB);
	ledcWrite(_ledChannelR, 0);
	ledcWrite(_ledChannelG, 0);
	ledcWrite(_ledChannelB, 0);
	strcpy(_currentState,"0");
	Logger.debug("RgbLed", "Constructor");
}

void RgbLed::setActor(const char* newState)
{
	char message[LENGTH_LOGGER_MESSAGE];
	snprintf(message, LENGTH_LOGGER_MESSAGE, "setActor: %s", newState);
	Logger.debug("RgbLed", message);
	int value = atoi(newState);
	int r = int (value / 10000);
	value -= r*10000;
	r=(r*256)/100;
	int g = int (value / 100);
	value -= g * 100;
	g=(g*256)/100;
	int b = (int) (value*256/100);
	ledcWrite(_ledChannelR, r);
	ledcWrite(_ledChannelG, g);
	ledcWrite(_ledChannelB, b);
	strncpy(_currentState, newState, LENGTH_STATE-1);
}

