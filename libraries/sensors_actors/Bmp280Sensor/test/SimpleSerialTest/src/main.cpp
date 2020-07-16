#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>

#include <Bmp280Sensor.h>

const gpio_num_t SDA_GPIO = GPIO_NUM_21;
const gpio_num_t SCL_GPIO = GPIO_NUM_22;

const char *SERIAL_LOGGER_TAG = "SLT";

extern "C"
{
    void app_main(void);
}

void app_main()
{
    Logger.init("Bmp280Sensor Test");
    Logger.info("Bmp280Sensor Test, app_main()", "Start!");
  
    SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(serialLoggerTarget);

    Bmp280Sensor *sensor = new Bmp280Sensor(SDA_GPIO, SCL_GPIO);

    while (true)
    {
        printf("Pressure: %.2f Pa, Temperature: %.2f °C, Humidity: %.2f %%\n", sensor->getPressure(), sensor->getTemperature(), sensor->getHumidity());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
