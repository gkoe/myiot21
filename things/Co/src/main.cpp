#include <stdio.h>
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_log.h>

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

#include <Co.h>

extern "C"
{
  void app_main(void);
}

#include <Co.h>
const adc1_channel_t adcChannel = ADC1_CHANNEL_5;  // GPIO33

const char *SERIAL_LOGGER_TAG = "SLT";

bool startAsStation = true;

void app_main()
{
  printf("=========\n");
  printf("Co\n");
  printf("=========\n");
  Logger.init("Co");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  EspConfig.init();
  SystemService.init();
  Logger.info("Co, app_main()", "EspStation init!");
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
    if (EspStation.isStationOnline())
    {
      EspConfig.setNvsIntValue("startAp", 0);
      HttpServer.init();
      Logger.info("Co, app_main()", "HttpServer started");
      EspTime.init();
      EspUdp.init();
      UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
      Logger.addLoggerTarget(udpLoggerTargetPtr);
      EspMqttClient.init(EspConfig.getThingName());
    }
    else
    {
      Logger.info("Co, app_main()", "restart as accesspoint!");
      EspConfig.setNvsIntValue("startAp", 1);
      SystemService.restart();
    }
  }
  // Thing und Sensoren anlegen, auch wenn keine Netzwerkverbindung besteht.
  // Damit bleibt die CO2-Alarmierung aktiv.
  Thing.init();
  Logger.info("Co, app_main()", "Thing created");
  // >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  Co *coPtr = new Co(adcChannel, 500, "Cosensor", "Co", "", 20, 0, 5000, false);
  Thing.addSensor(coPtr);
  //<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil

  while (true)
  {
    vTaskDelay(1);
    SystemService.checkSystem();
  }
}
