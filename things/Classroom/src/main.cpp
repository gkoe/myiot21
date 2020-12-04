#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>
#include <driver/adc.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspAp.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>
#include <Thing.h>
#include <Mhz.h>
#include <SimplePir.h>
#include <Bmp280Sensor.h>
#include <Bmp280_Humidity.h>
#include <Bmp280_Temperature.h>
#include <Bmp280_Pressure.h>
#include <Noise.h>
#include <Luminosity.h>
#include <TrafficLight.h>

// NeoPixels
// static const gpio_num_t NEOPIXEL_GPIO = GPIO_NUM_27;

// Luminosity mit TSL2561
#define LUMINOSITY_I2C_SDA GPIO_NUM_18
#define LUMINOSITY_I2C_SCL GPIO_NUM_19

// Noise Microphone
const adc1_channel_t adcChannelMicrophone = ADC1_CHANNEL_5; // GPIO33

//BMP280
const gpio_num_t BMP280_SDA_GPIO = GPIO_NUM_21;
const gpio_num_t BMP280_SCL_GPIO = GPIO_NUM_22;

// PIR
// const gpio_num_t PIR_PIN = GPIO_NUM_25;

/// MHZ
// const gpio_num_t CO2_TXD = GPIO_NUM_12;
// const gpio_num_t CO2_RXD = GPIO_NUM_13;
// // #define CO2_TXD (GPIO_NUM_12) // verbunden mit Rx am Mhz
// // #define CO2_RXD (GPIO_NUM_13) // verbunden mit Tx am Mhz

// http://192.168.10.1/setconfig?ssid=SSID&password=PASSWORD
// http://192.168.10.1/setconfig?ntpserver=192.168.0.2
// http://192.168.10.1/setconfig?mqttbroker=192.168.0.122&mqttport=1883
// http://192.168.10.1/setconfig?thingname=co2demo

extern "C"
{
  void app_main(void);
}

const char *SERIAL_LOGGER_TAG = "SLT";

bool startAsStation = true;

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  // Mhz *mhzPtr = nullptr;
  // TrafficLight *trafficLight = nullptr;
  printf("=========\n");
  printf("Classroom\n");
  printf("=========\n");
  Logger.init("Classroom");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  EspConfig.init();
  SystemService.init();
  Logger.info("Classroom, app_main()", "EspStation init!");
  int isStartAp = EspConfig.getNvsIntValue("startAp");

  if (isStartAp == 1)
  {
    startAsStation = false;
    EspConfig.setNvsIntValue("startAp", 0);
    EspAp.init();
    while (!EspAp.isApStarted())
    {
      vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    Logger.info("ConfigEspViaAp", "Connect with AP from ESP_xxx");
    HttpServer.init();
    Logger.info("ConfigEspViaAp", "After HttpServer init");
  }
  else
  {
    EspStation.init();
    // long rounds = 0;
    // Logger.info("Classroom, app_main()", "Waiting 5 seconds for connection!");
    // while (!EspStation.isStationOnline() && rounds < 5000)
    // {
    //   vTaskDelay(1 / portTICK_PERIOD_MS);
    //   Logger.info("Classroom, app_main()", "Waiting!!!!");
    //   rounds++;
    // }
    if (EspStation.isStationOnline())
    {
      EspConfig.setNvsIntValue("startAp", 0);
      HttpServer.init();
      Logger.info("Classroom, app_main()", "HttpServer started");
      EspTime.init();
      EspUdp.init();
      UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
      Logger.addLoggerTarget(udpLoggerTargetPtr);
      EspMqttClient.init(EspConfig.getThingName());
    }
    else
    {
      Logger.info("Classroom, app_main()", "restart as accesspoint!");
      EspConfig.setNvsIntValue("startAp", 1);
      SystemService.restart();
    }
  }
  // Thing und Sensoren anlegen, auch wenn keine Netzwerkverbindung besteht.
  // Damit bleibt die CO2-Alarmierung aktiv.
  Thing.init();
  Logger.info("Classroom, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  // mhzPtr = new Mhz(CO2_RXD, CO2_TXD, EspConfig.getThingName(), "co2", "ppm", 5.0, 300, 5000);
  // Thing.addSensor(mhzPtr);
  // PIR-Sensor mit 10 Sekunden Karenzzeit
  // IotSensor *pirPtr = new SimplePir(PIR_PIN, 5, EspConfig.getThingName(), "motion", "");
  // Thing.addSensor(pirPtr);
  Bmp280Sensor *sensorPtr = new Bmp280Sensor(BMP280_SDA_GPIO, BMP280_SCL_GPIO);
  IotSensor *temperatureSensorPtr =
      new Bmp280_Temperature(sensorPtr, EspConfig.getThingName(), "temperature", "Grad", 0.2, 9.9, 49.9);
  Thing.addSensor(temperatureSensorPtr);
  IotSensor *humiditySensorPtr =
      new Bmp280_Humidity(sensorPtr, EspConfig.getThingName(), "humidity", "%", 0.2, 0.0, 100.0);
  Thing.addSensor(humiditySensorPtr);
  IotSensor *pressureSensorPtr =
      new Bmp280_Pressure(sensorPtr, EspConfig.getThingName(), "pressure", "hPa", 1.0, 800.0, 1100.0);
  Thing.addSensor(pressureSensorPtr);
  IotSensor *noisePtr = new Noise(adcChannelMicrophone, 1000, EspConfig.getThingName(), "noise", "", 80, 0, 5000, false);
  Thing.addSensor(noisePtr);
  Luminosity *luminosityPtr = new Luminosity(LUMINOSITY_I2C_SDA, LUMINOSITY_I2C_SCL, EspConfig.getThingName(), "luminosity", "Lux", 10, 0, 50000, true);
  Thing.addSensor(luminosityPtr);

  // trafficLight = new TrafficLight(NEOPIXEL_GPIO, EspConfig.getThingName(), "trafficlight");
  // Thing.addActor(trafficLight);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  char co2LimitText[LENGTH_SHORT_TEXT];
  int round = 0;
  while (true)
  {
    vTaskDelay(1);
    SystemService.checkSystem();
    // Sensoren auch abfragen, wenn keine Netzwerkverbindung besteht
    Thing.refreshSensorsAndActors();
    // int co2Yellow = 500;
    // int co2Red = 1500;
    // EspConfig.getNvsStringValue("co2yellow", co2LimitText);
    // if (strlen(co2LimitText) > 0)
    // {
    //   co2Yellow = atoi(co2LimitText);
    // }
    // EspConfig.getNvsStringValue("co2red", co2LimitText);
    // if (strlen(co2LimitText) > 0)
    // {
    //   co2Red = atoi(co2LimitText);
    // }
    // // sprintf(loggerMessage, "Limit for co2Yellow: %d", co2Yellow);
    // // Logger.info("Classroom, loop()", loggerMessage);
    // float co2 = mhzPtr->getLastMeasurement();
    // char trafficLightNew[LENGTH_STATE];
    // if (co2 > co2Red)
    // {
    //   strcpy(trafficLightNew, "4");
    // }
    // else if (co2 > co2Yellow)
    // {
    //   strcpy(trafficLightNew, "2");
    // }
    // else
    // {
    //   strcpy(trafficLightNew, "1");
    // }
    // if (strcmp(trafficLightNew, trafficLight->getCurrentState()) != 0)
    // {
    //   trafficLight->setState(trafficLightNew);
    //   Logger.info("Classroom, loop(), trafficLight new: ", trafficLightNew);
    // }
    // round++;
    // if (round > 200)
    // {
    //   sprintf(loggerMessage, "co2: %.2f, Trafficlight current state: %s, new state: %s, limit for yellow: %d, limit for red: %d",
    //           co2, trafficLight->getCurrentState(), trafficLightNew, co2Yellow, co2Red);
    //   Logger.info("Classroom, loop(), Round ", loggerMessage);
    //   round = 0;
    // }
  }
}
