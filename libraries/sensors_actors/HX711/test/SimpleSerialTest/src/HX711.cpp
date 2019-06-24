#include <HX711.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>

const int OFFSET = 0;
const float SCALE = 1.0;


uint8_t shiftInSlow(gpio_num_t dataPin, gpio_num_t clockPin)
{
    uint8_t value = 0;
    uint8_t i;

    for (i = 0; i < 8; ++i)
    {
        gpio_set_level(clockPin, 1);
        ets_delay_us(1);
        value |= gpio_get_level(dataPin) << (7 - i);
        gpio_set_level(clockPin, 0);
        ets_delay_us(1);
    }
    return value;
}

bool isReady(gpio_num_t dataPin) {
	return gpio_get_level(dataPin) == 0;
}

long read(gpio_num_t dataPin, gpio_num_t clockPin) {
	// wait for the chip to become ready
	while (!isReady(dataPin)) {
        vTaskDelay(1);
	}

	unsigned long value = 0;
	uint8_t data[3] = { 0 };
	uint8_t filler = 0x00;

	// pulse the clock pin 24 times to read the data
	data[2] = shiftInSlow(dataPin,clockPin);
	data[1] = shiftInSlow(dataPin,clockPin);
	data[0] = shiftInSlow(dataPin,clockPin);

    int GAIN = 1;

	// set the channel and the gain factor for the next reading using the clock pin
	for (unsigned int i = 0; i < GAIN; i++) {
        gpio_set_level(clockPin, 1);
        ets_delay_us(1);
        gpio_set_level(clockPin, 0);
        ets_delay_us(1);
	}

	// Replicate the most significant bit to pad out a 32-bit signed integer
	if (data[2] & 0x80) {
		filler = 0xFF;
	} else {
		filler = 0x00;
	}

	// Construct a 32-bit signed integer
	value = ( static_cast<unsigned long>(filler) << 24
			| static_cast<unsigned long>(data[2]) << 16
			| static_cast<unsigned long>(data[1]) << 8
			| static_cast<unsigned long>(data[0]) );

	return static_cast<long>(value);
}

/**
 * Ermittelt alle Sekunden die aktuelle Leistung.
 */
void measureInLoopTask(void *pvParameter)
{
    HX711 *hx711Ptr = (HX711 *)pvParameter;
    while (1)
    {
        long value = read(hx711Ptr->_dataPin, hx711Ptr->_clockPin);
        hx711Ptr->_actWeight = (value-OFFSET) / SCALE;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

HX711::HX711(gpio_num_t dataPin, gpio_num_t clockPin, const char *thingName, const char *name, const char *unit, float threshold)
    : IotSensor(thingName, name, unit, threshold)
{
    _dataPin = dataPin;
    _clockPin = clockPin;
    gpio_set_direction(dataPin, GPIO_MODE_INPUT);
    gpio_set_direction(clockPin, GPIO_MODE_OUTPUT);
    gpio_set_level(clockPin, 0);

    xTaskCreate(measureInLoopTask,   /* Task function. */
                "measureInLoopTask", /* String with name of task. */
                4096,                /* Stack size in words. */
                this,                /* Parameter passed as input of the task */
                1,                   /* Priority of the task. */
                NULL                 /* Task handle. */
    );
}

void HX711::measure()
{
    setMeasurement(_actWeight);
}