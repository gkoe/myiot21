#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/adc.h>

#include <Template.h>

#include <EspConfig.h>
#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <SystemService.h>
#include <Thing.h>


const char *SERIAL_LOGGER_TAG = "SLT";

const adc1_channel_t adcChannel = ADC1_CHANNEL_5;  // GPIO33

#define I2C_SDA        GPIO_NUM_18
#define I2C_SCL        GPIO_NUM_19

#define TRIGGER_GPIO  GPIO_NUM_18
#define ECHO_GPIO     GPIO_NUM_19

#define UART_TXD GPIO_NUM_25
#define UART_RXD GPIO_NUM_33


extern "C"
{
  void app_main(void);
}

void app_main()
{
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  printf("================\n");
  printf("TemplateSensor-Test\n");
  printf("================\n");
  EspConfig.init();
  Logger.init("TemplateSensorTest");
  SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_ERROR);
  Logger.addLoggerTarget(serialLoggerTarget);
  SystemService.init();
  Logger.info("TemplateSensorTest;app_main()", "Initialize Templatesensor!");

  Template *templatePtr = new Template(adcChannel, "Templatesensor", "Template", "", 20, 0, 5000, false);

  while (true)
  {
    templatePtr->measure();
    float templateValue = templatePtr->getLastMeasurement();
    sprintf(loggerMessage, "Template:  %.1f", templateValue);
    Logger.info("TemplateSensorTest;app_main()", loggerMessage);
    vTaskDelay(1000 / portTICK_RATE_MS);
    SystemService.checkSystem();
  }
}



