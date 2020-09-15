#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <neopixel.h>
#include "esp_event_loop.h" //	for usleep

#include <TrafficLight.h>
#include <Thing.h>
#include <Logger.h>
#include <esp_log.h>

#define NR_LED 8
#define NEOPIXEL_RMT_CHANNEL RMT_CHANNEL_2
uint8_t intensities[] = {0, 5, 20, 255, 20, 5, 0, 0};

pixel_settings_t __px;
pixel_timing_t __px_timings;
//int __intensity = 20;
int __intensity = 249;

uint32_t __pixels[NR_LED];

// void setPixelValue(int index, uint8_t red, uint8_t green, uint8_t blue)
// {
// 	uint32_t color = green << 16 + red << 8 + blue;
// 	__px.pixels[index] = color;
// }

void setTrafficLightTask(void *pvParameter)
{
	printf("Start setTrafficLightTask()\n");
	TrafficLight *trafficLightPtr = (TrafficLight *)pvParameter;
	// int increment = 1;
	char oldState[LENGTH_STATE];
	strcpy(oldState, trafficLightPtr->getCurrentState());
	while (1)
	{
		// int intensityTime = (500 / __intensity) + 5;
		// usleep(1000 * intensityTime);
		// vTaskDelay(3 * intensityTime);
		if (strcmp(oldState, trafficLightPtr->getCurrentState()) != 0)
		{
			for (int j = 0; j < NR_LED; j++)
			{
				np_set_pixel_rgbw(&__px, j, trafficLightPtr->_red, trafficLightPtr->_green, trafficLightPtr->_blue, 0);
				// setPixelValue(j, intensity, 0, 0);
				// printf("Pixelcount: %d, pixelNum: %d, red: %d \n", __px.pixel_count, j, intensity);
			}
			np_show(&(__px), NEOPIXEL_RMT_CHANNEL);
			strcpy(oldState, trafficLightPtr->getCurrentState());
		}
		vTaskDelay(100);
		// int redIntensity = (__intensity * trafficLightPtr->_red) / 250;
		// int greenIntensity = (__intensity * trafficLightPtr->_green) / 250;
		// int blueIntensity = (__intensity * trafficLightPtr->_blue) / 250;
		// // printf("Intensity (R:G:B): %d:%d:%d, Delay: %d\n", redIntensity, greenIntensity, blueIntensity, intensityTime);
		// for (int j = 0; j < NR_LED; j++)
		// {
		// 	np_set_pixel_rgbw(&__px, j, redIntensity, greenIntensity, blueIntensity, 0);
		// 	// setPixelValue(j, intensity, 0, 0);
		// 	// printf("Pixelcount: %d, pixelNum: %d, red: %d \n", __px.pixel_count, j, intensity);
		// }
		// // printf("np_show()\n");
		// np_show(&(__px), NEOPIXEL_RMT_CHANNEL);
		// __intensity += increment;
		// if (__intensity <= 20 || __intensity >= 250)
		// {
		// 	increment *= -1;
		// }
	}
}

TrafficLight::TrafficLight(const gpio_num_t pin, const char *thingName, const char *name) : IotActor(thingName, name)
{
	_pin = pin;

	int i;
	int rc;

	rc = neopixel_init(pin, NEOPIXEL_RMT_CHANNEL); //!
	ESP_LOGE("main", "neopixel_init rc = %d", rc);
	vTaskDelay(1000);
	// usleep(1000 * 1000);

	for (i = 0; i < NR_LED; i++)
	{
		__pixels[i] = 0;
	}
	__px.pixels = (uint8_t *)__pixels;
	__px.pixel_count = NR_LED;
	strcpy(__px.color_order, "GRB");

	__px.nbits = 24;
	__px.brightness = 0x80;

	// memset(&px.timings, 0, sizeof(px.timings));
	__px_timings.mark.level0 = 1;
	__px_timings.space.level0 = 1;
	__px_timings.mark.duration0 = 12;
	__px_timings.mark.duration1 = 14;
	__px_timings.space.duration0 = 7;
	__px_timings.space.duration1 = 16;
	__px_timings.reset.duration0 = 600;
	__px_timings.reset.duration1 = 600;

	__px.timings = __px_timings;

	np_show(&__px, NEOPIXEL_RMT_CHANNEL);

	setState("0");  // Initialisierung
	xTaskCreate(setTrafficLightTask,   /* Task function. */
				"setTrafficLightTask", /* String with name of task. */
				4096,				   /* Stack size in words. */
				this,				   /* Parameter passed as input of the task */
				1,					   /* Priority of the task. */
				NULL				   /* Task handle. */
	);
}

void TrafficLight::setActor(const char *newState)
{
	bool isGreen = (strcmp(newState, "GREEN") == 0) || (strcmp(newState, "green") == 0) || (strcmp(newState, "1") == 0);
	bool isYellow = (strcmp(newState, "YELLOW") == 0) || (strcmp(newState, "yellow") == 0) || (strcmp(newState, "2") == 0);
	bool isOrange = (strcmp(newState, "ORANGE") == 0) || (strcmp(newState, "orange") == 0) || (strcmp(newState, "3") == 0);
	bool isRed = (strcmp(newState, "RED") == 0) || (strcmp(newState, "red") == 0) || (strcmp(newState, "4") == 0);
	_isOff = (strcmp(newState, "OFF") == 0) || (strcmp(newState, "off") == 0) || (strcmp(newState, "0") == 0);
	char newStateNormed[LENGTH_STATE];
	if (isGreen)
	{
		_red = 0;
		_green = 10;
		_blue = 0;
		strcpy(newStateNormed, "1");
	}
	else if (isYellow)
	{
		_red = 120;
		_green = 120;
		_blue = 0;
		strcpy(newStateNormed, "2");
	}
	else if (isOrange)
	{
		_red = 255;
		_green = 165;
		_blue = 0;
		strcpy(newStateNormed, "3");
	}
	else if (isRed)
	{
		_red = 255;
		_green = 0;
		_blue = 0;
		strcpy(newStateNormed, "4");
	}
	else if (_isOff)
	{
		_red = 0;
		_green = 0;
		_blue = 0;
		strcpy(newStateNormed, "0");
	}
	if (strcmp(newStateNormed, getCurrentState()) == 0) // State nicht verändert
		return;

	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	sprintf(loggerMessage, "Newstate: %s , Actstate: %s",
			newStateNormed, getCurrentState());

	Logger.info("TrafficLight;setActor()", loggerMessage);
	setCurrentState(newStateNormed);
	Logger.info("TrafficLight;setCurrentState(), currentState: ", getCurrentState());
}
