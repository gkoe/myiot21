
#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ultrasonic.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>

#include <HX711.h>


const char *SERIAL_LOGGER_TAG = "SLT";

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 23;
const int LOADCELL_SCK_PIN = 22;

// 2. Adjustment settings
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;

const float DIVIDER = -4.1; //-0.55;

extern "C"
{
  void app_main(void);
}

void app_main()
{
  // initArduino();
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("===============\n");
  printf("HX711-Test\n");
  printf("===============\n");
  EspConfig.init();
  Logger.init("HX711Test");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("HX711Test;app_main()", "Initialize HX711!");

  HX711 *hx711 = new HX711(TRIGGER_GPIO, GPIO_NUM_19, "Ultrasonic", "waterlevel", "cm", 0.1);
  hx711->begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  while (true)
  {
    float cm = ultrasonic->getAverageDistance();
    sprintf(loggerMessage, "Distance:  %.1f", cm);
    Logger.info("UltrasonicTest;app_main()", loggerMessage);
    vTaskDelay(5000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}
