#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>
#include <Thing.h>

#include <Ds18b20Sensor.h>
#include <Ds18b20Hub.h>
static const gpio_num_t SENSOR_GPIO = GPIO_NUM_18;
static const char DS18B20_Address[] = "910114591607aa28";  

extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("==============\n");
  printf("Thing with Ds18b20\n");
  printf("==============\n");
  Logger.init("Ds18b20Thing");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  EspConfig.init();
  SystemService.init();
  EspStation.init();
  Logger.info("Ds18b20Thing, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("Ds18b20Thing, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(EspConfig.getThingName());
  Thing.init();
  Logger.info("Ds18b20Thing, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Ds18b20Hub *ds18b20Hub = new Ds18b20Hub(SENSOR_GPIO);
  Ds18b20Sensor *ds18b20 = new Ds18b20Sensor(ds18b20Hub, DS18B20_Address,  "DS18B20", "temperature", "°", 0.1);
  Thing.addSensor(ds18b20);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
