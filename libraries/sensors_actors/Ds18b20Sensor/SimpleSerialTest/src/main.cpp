#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <EspConfig.h>
#include <Logger.h>
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>

#include <Ds18b20Sensor.h>
#include <Ds18b20Hub.h>

const char *SERIAL_LOGGER_TAG = "SLT";

static const gpio_num_t SENSOR_GPIO = GPIO_NUM_18;
static const char DS18B20_Address[] = "910114591607aa28";

extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("=====================\n");
  printf("Ds18b20Sensor-Test\n");
  printf("=====================\n");
  EspConfig.init();
  Logger.init("Ds18b20SensorTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("Ds18b20SensorTest;app_main()", "Initialize Ds18b20Sensor!");

  Ds18b20Hub *ds18b20Hub = new Ds18b20Hub(SENSOR_GPIO);
  Ds18b20Sensor *ds18b20 = new Ds18b20Sensor(ds18b20Hub, DS18B20_Address,  "TestThing", "Ds18b20", "°", 0.1);

  while (true)
  {
    sprintf(loggerMessage, "Ds18b20 measure");
    Logger.info("Ds18b20SensorTest;app_main()", loggerMessage);
    ds18b20->measure();
    float temp = ds18b20->getLastMeasurement();
    sprintf(loggerMessage, "Ds18b20 Temperature:  %.1f", temp);
    Logger.info("Ds18b20SensorTest;app_main()", loggerMessage);
    vTaskDelay(3000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



