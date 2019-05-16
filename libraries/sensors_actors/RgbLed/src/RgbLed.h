#pragma once

#include <IotActor.h>


/*
	RgbLed ist ein Aktor, dem über einen Floatvalue die Helligkeit für Rot, Grün und
	Blau so übermittelt werden, dass die Einer/Zehnerstelle Blau in %, die Hunderter/Tausenderstelle Grün
	und die Zehntausender/Hunderttausenderstelle Rot in Prozent angibt.
	Die Helligkeit wird über PWM gesteuert
*/
class RgbLed : public IotActor
{
public:
	RgbLed(const int pinR, const int pinG, const int pinB, 
			const char *thingName, const char *name);

	virtual void setActor(const char* newState);

	const char *Red = "990000";
	const char *Green = "9900";
	const char *Blue = "99";
	const char *Off = "00";

private:
	int _pinR;
	int _pinG;
	int _pinB;
	int _ledChannelR = 1;	// IO-Pin wird per PWM angesteuert
	int _ledChannelG = 2;
	int _ledChannelB = 3;
};
