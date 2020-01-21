#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include "driver/gpio.h"
#include <driver/adc.h>

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

#include <MultiPir.h>
#include <RgbLed.h>
#include <Dht22.h>
#include <Dht22_Temperature.h> // Wrapper über den Dht22 zur Einbindung als Temperatursensor
#include <Dht22_Humidity.h>
#include <Mhz.h>
#include <Noise.h>
#include <Luminosity.h>

// MHZ CO2 
#define UART_TXD (GPIO_NUM_12)  // verbunden mit Rx am Mhz
#define UART_RXD (GPIO_NUM_13)  // verbunden mit Tx am Mhz

// Noise
const adc1_channel_t adcChannel = ADC1_CHANNEL_5;  // GPIO33

// Luminosity 
#define I2C_SDA        GPIO_NUM_18
#define I2C_SCL        GPIO_NUM_19

// MultiPir
gpio_num_t PIR_PINS[] = {GPIO_NUM_25,GPIO_NUM_26,GPIO_NUM_27,GPIO_NUM_14 };

// Dht22
gpio_num_t DHT22_PIN = GPIO_NUM_5;

extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

void app_main()
{
  printf("================================================\n");
  printf("Tadeot with RgbLed, Dht22, Noise, Motion and Pir\n");
  printf("================================================\n");
  EspConfig.init();
  const char *thingName = EspConfig.getThingName();
  Logger.init("Tadeot");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  EspStation.init();
  Logger.info("Tadeot, app_main()", "Waiting for connection!");
  while (!EspStation.isStationOnline())
  {
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }
  HttpServer.init();
  Logger.info("Tadeot, app_main()", "HttpServer started");
  EspTime.init();
  EspUdp.init();
  UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_INFO);
  Logger.addLoggerTarget(udpLoggerTargetPtr);
  EspMqttClient.init(thingName);
  Thing.init();
  Logger.info("Tadeot, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Dht22 *dhtSensorPtr = new Dht22();
  dhtSensorPtr->init(DHT22_PIN);
  IotSensor *temperatureSensorPtr =
      new Dht22_Temperature(dhtSensorPtr, thingName, "temperature", "Grad", 0.1, 15.0, 50.0);
  Thing.addSensor(temperatureSensorPtr);
  IotSensor *humiditySensorPtr =
      new Dht22_Humidity(dhtSensorPtr, thingName, "humidity", "%", 0.5, 0.0, 100.0);
  Thing.addSensor(humiditySensorPtr);
  IotSensor *noisePtr = new Noise(adcChannel, 1000, thingName, "noise", "", 200, 0, 5000, false);
  Thing.addSensor(noisePtr);
  IotSensor *luminosityPtr = new Luminosity(I2C_SDA, I2C_SCL, thingName, "luminosity", "Lux", 10, 0, 50000, false);
  Thing.addSensor(luminosityPtr);
  IotSensor *multiPirPtr = new MultiPir(PIR_PINS, 4, 60, thingName, "motion", "", 0.5, 0, 1111, false);
  Thing.addSensor(multiPirPtr);
  Mhz *mhzPtr = new Mhz(UART_RXD, UART_TXD, thingName, "co2", "ppm", 5.0, 300.0, 5000.0, true);
  Thing.addSensor(mhzPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    SystemService.checkSystem();
    Thing.refreshSensorsAndActors();
    vTaskDelay(1);
  }
}
