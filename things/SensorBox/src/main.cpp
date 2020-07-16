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

// Luminosity mit TSL2561
#define I2C_SDA GPIO_NUM_18
#define I2C_SCL GPIO_NUM_19

// Noise Microphone
const adc1_channel_t adcChannel = ADC1_CHANNEL_5; // GPIO33

//BMP280
const gpio_num_t SDA_GPIO = GPIO_NUM_21;
const gpio_num_t SCL_GPIO = GPIO_NUM_22;

// PIR
const gpio_num_t PIR_PIN = GPIO_NUM_25;

/// MHZ
#define UART_TXD (GPIO_NUM_12) // verbunden mit Rx am Mhz
#define UART_RXD (GPIO_NUM_13) // verbunden mit Tx am Mhz

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
  printf("=========\n");
  printf("Sensorbox\n");
  printf("=========\n");
  Logger.init("SensorBox");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  EspConfig.init();
  SystemService.init();
  Logger.info("SensorBox, app_main()", "EspStation init!");
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
  }
  else
  {
    EspStation.init();
    // long rounds = 0;
    // Logger.info("SensorBox, app_main()", "Waiting 5 seconds for connection!");
    // while (!EspStation.isStationOnline() && rounds < 5000)
    // {
    //   vTaskDelay(1 / portTICK_PERIOD_MS);
    //   Logger.info("SensorBox, app_main()", "Waiting!!!!");
    //   rounds++;
    // }
    if (EspStation.isStationOnline())
    {
      EspConfig.setNvsIntValue("startAp", 0);
      HttpServer.init();
      Logger.info("SensorBox, app_main()", "HttpServer started");
      EspTime.init();
      EspUdp.init();
      UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
      Logger.addLoggerTarget(udpLoggerTargetPtr);
      EspMqttClient.init(EspConfig.getThingName());
      Thing.init();
      Logger.info("SensorBox, app_main()", "Thing created");
      // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
      Mhz *mhzPtr = new Mhz(UART_RXD, UART_TXD, EspConfig.getThingName(), "co2", "ppm", 5.0, 300, 5000);
      Thing.addSensor(mhzPtr);
      // PIR-Sensor mit 10 Sekunden Karenzzeit
      IotSensor *pirPtr = new SimplePir(PIR_PIN, 5, EspConfig.getThingName(), "motion", "");
      Thing.addSensor(pirPtr);
      Bmp280Sensor *sensorPtr = new Bmp280Sensor(SDA_GPIO, SCL_GPIO);
      IotSensor *temperatureSensorPtr =
          new Bmp280_Temperature(sensorPtr, EspConfig.getThingName(), "temperature", "Grad", 0.1, 9.9, 49.9);
      Thing.addSensor(temperatureSensorPtr);
      IotSensor *humiditySensorPtr =
          new Bmp280_Humidity(sensorPtr, EspConfig.getThingName(), "humidity", "%", 0.2, 0.0, 100.0);
      Thing.addSensor(humiditySensorPtr);
      IotSensor *pressureSensorPtr =
          new Bmp280_Pressure(sensorPtr, EspConfig.getThingName(), "pressure", "hPa", 1.0, 800.0, 1100.0);
      Thing.addSensor(pressureSensorPtr);
      IotSensor *noisePtr = new Noise(adcChannel, 1000, EspConfig.getThingName(), "noise", "", 40, 0, 5000, false);
      Thing.addSensor(noisePtr);
      Luminosity *luminosityPtr = new Luminosity(I2C_SDA, I2C_SCL, EspConfig.getThingName(), "luminosity", "Lux", 10, 0, 50000, true);
      Thing.addSensor(luminosityPtr);

      //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil
    }
    else
    {
      Logger.info("SensorBox, app_main()", "restart as accesspoint!");
      EspConfig.setNvsIntValue("startAp", 1);
      SystemService.restart();
    }
  }

  while (true)
  {
    SystemService.checkSystem();
    vTaskDelay(1);
    if (startAsStation)
    {
      Thing.refreshSensorsAndActors();
    }
  }
}
