#pragma once

#include <IotActor.h>
#include "driver/gpio.h"
#include <neopixel.h>



class TrafficLight : public IotActor
{
public:
	TrafficLight(const gpio_num_t pin, const char *thingName, const char *name);

	// pixel_settings_t px;
	bool _isOff = true;
	uint8_t _red = 0;
	uint8_t _green = 0;
	uint8_t _blue = 0;

	// GREEN, YELLOW, ORANGE, RED, OFF
	virtual void setActor(const char* newState);

private:
	gpio_num_t _pin;
};
