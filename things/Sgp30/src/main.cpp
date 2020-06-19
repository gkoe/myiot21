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
#include <Air_Sgp30.h>
#include <Sgp30_Co2.h>
#include <Sgp30_Tvoc.h>

extern "C"
{
  void app_main(void);
}

const gpio_num_t I2C_SDA = GPIO_NUM_18;
const gpio_num_t I2C_SCL = GPIO_NUM_19;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("================\n");
  printf("Thing with SGP30\n");
  printf("================\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("ThingTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("ThingTest, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("ThingTest, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(thingName);
  Thing.init();
  Logger.info("ThingTest, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Air_Sgp30 *sgp30SensorPtr = new Air_Sgp30(I2C_SDA, I2C_SCL, "Sgp30sensor", "Sgp30", "", 20);
  IotSensor *co2SensorPtr =
      new Sgp30_Co2(sgp30SensorPtr, thingName, "co2", "ppm", 20, 300, 6000);
  Thing.addSensor(co2SensorPtr);
  IotSensor *tvocSensorPtr =
      new Sgp30_Tvoc(sgp30SensorPtr, thingName, "tvoc", "ppb", 20, 0, 5000);
  Thing.addSensor(tvocSensorPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
