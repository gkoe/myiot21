#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "sdkconfig.h"

#include "smbus.h"
#include "tsl2561.h"
#include "Luminosity.h"
#include <sys/time.h>

#include <Logger.h>

#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_TX_BUF_LEN 0 // disabled
#define I2C_MASTER_RX_BUF_LEN 0 // disabled
#define I2C_MASTER_FREQ_HZ 100000
// #define I2C_MASTER_SDA_IO        18
// #define I2C_MASTER_SCL_IO        19
#define CONFIG_TSL2561_I2C_ADDRESS 0x39

void tsl2561_task(void *pvParameter)
{
  Luminosity *luminosityPtr = (Luminosity *)pvParameter;
  // Set up I2C
  i2c_port_t i2c_num = I2C_MASTER_NUM;
  uint8_t address = CONFIG_TSL2561_I2C_ADDRESS;

  // Set up the SMBus
  smbus_info_t *smbus_info = smbus_malloc();
  smbus_init(smbus_info, i2c_num, address);
  smbus_set_timeout(smbus_info, 1000 / portTICK_RATE_MS);

  // Set up the TSL2561 device
  tsl2561_info_t *tsl2561_info = tsl2561_malloc();
  tsl2561_init(tsl2561_info, smbus_info);

  // Set sensor integration time and gain
  tsl2561_set_integration_time_and_gain(tsl2561_info, TSL2561_INTEGRATION_TIME_402MS, TSL2561_GAIN_1X);
  //tsl2561_set_integration_time_and_gain(tsl2561_info, TSL2561_INTEGRATION_TIME_402MS, TSL2561_GAIN_16X);
  int lux;
  while (1)
  {
    tsl2561_visible_t visible = 0;
    tsl2561_infrared_t infrared = 0;
    tsl2561_read(tsl2561_info, &visible, &infrared);

    printf("\nFull spectrum: %d\n", visible + infrared);
    printf("Infrared:      %d\n", infrared);
    printf("Visible:       %d\n", visible);
    lux = tsl2561_compute_lux(tsl2561_info, visible, infrared);
    printf("Lux:           %d\n", lux);

    luminosityPtr->_actLux = lux;

    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

Luminosity::Luminosity(gpio_num_t sda, gpio_num_t scl, const char *thingName, const char *name, const char *unit, float threshold, float minValue, float maxValue, bool getAverageValue)
    : IotSensor(thingName, name, unit, threshold, minValue, maxValue, getAverageValue)
{
  _sda = sda;
  _scl = scl;

  i2c_port_t i2c_master_port = I2C_MASTER_NUM;
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = sda;
  conf.sda_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
  conf.scl_io_num = scl;
  conf.scl_pullup_en = GPIO_PULLUP_DISABLE; // GY-2561 provides 10kΩ pullups
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  i2c_param_config(i2c_master_port, &conf);
  i2c_driver_install(i2c_master_port, conf.mode,
                     I2C_MASTER_RX_BUF_LEN,
                     I2C_MASTER_TX_BUF_LEN, 0);
  xTaskCreate(&tsl2561_task, "tsl2561_task", 2048, this, 5, NULL);

  // xTaskCreate(measureLuminosityInLoopTask,   /* Task function. */
  //             "measurePowerInLoopTask", /* String with name of task. */
  //             4096,                /* Stack size in words. */
  //             this,                /* Parameter passed as input of the task */
  //             1,                   /* Priority of the task. */
  //             NULL                 /* Task handle. */
  // );
}

/**
  measure() gets the measurement and set it.
*/
void Luminosity::measure()
{
  setMeasurement(_actLux);
}
