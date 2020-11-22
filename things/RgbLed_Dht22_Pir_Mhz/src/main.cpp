#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "driver/gpio.h"

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
#include <RgbLed.h>
#include <Dht22.h>
#include <Dht22_Temperature.h> // Wrapper über den Dht22 zur Einbindung als Temperatursensor
#include <Dht22_Humidity.h>
#include <SimplePir.h>
#include <Mhz.h>

#define UART_TXD (GPIO_NUM_25)  // verbunden mit Rx am Mhz
#define UART_RXD (GPIO_NUM_33)  // verbunden mit Tx am Mhz

extern "C"
{
  void app_main(void);
}

const gpio_num_t DHT22_PIN = GPIO_NUM_27;
const gpio_num_t PIR_PIN = GPIO_NUM_26;
const gpio_num_t R = GPIO_NUM_13;
const gpio_num_t G = GPIO_NUM_12;
const gpio_num_t B = GPIO_NUM_14;

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("================================\n");
  printf("Thing with RgbLed, Dht22 and Pir\n");
  printf("================================\n");
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
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_INFO);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(thingName);
  Thing.init();
  Logger.info("ThingTest, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Dht22 *dhtSensorPtr = new Dht22();
  dhtSensorPtr->init(DHT22_PIN);
  IotSensor *temperatureSensorPtr =
      new Dht22_Temperature(dhtSensorPtr, thingName, "temperature", "Grad", 0.1, 15.0, 50.0);
  Thing.addSensor(temperatureSensorPtr);
  IotSensor *humiditySensorPtr =
      new Dht22_Humidity(dhtSensorPtr, thingName, "humidity", "%", 0.5, 0.0, 100.0);
  Thing.addSensor(humiditySensorPtr);
  IotSensor *pirPtr = new SimplePir(PIR_PIN, 60, thingName, "pir", "");
  Thing.addSensor(pirPtr);
  Mhz *mhzPtr = new Mhz(UART_RXD, UART_TXD, "Esp", "co2", "ppm", 5.0, 300.0, 5000.0, true);
  Thing.addSensor(mhzPtr);
  IotActor *rgbLedPtr =
      new RgbLed(R, G, B, thingName, "rgbled");
  Thing.addActor(rgbLedPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
