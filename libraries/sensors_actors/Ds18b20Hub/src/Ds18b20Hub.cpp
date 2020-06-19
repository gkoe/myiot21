#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <sys/time.h>
#include <string.h>

#include <Ds18b20Hub.h>
static const uint32_t LOOP_DELAY_MS = 1000;

void ds18x20_test(void *pvParameter)
{
  Ds18b20Hub *ds18b20Hub = (Ds18b20Hub *)pvParameter;

  // float temps[MAX_SENSORS];

  // There is no special initialization required before using the ds18x20
  // routines.  However, we make sure that the internal pull-up resistor is
  // enabled on the GPIO pin so that one can connect up a sensor without
  // needing an external pull-up (Note: The internal (~47k) pull-ups of the
  // ESP8266 do appear to work, at least for simple setups (one or two sensors
  // connected with short leads), but do not technically meet the pull-up
  // requirements from the ds18x20 datasheet and may not always be reliable.
  // For a real application, a proper 4.7k external pull-up resistor is
  // recommended instead!)

  while (true)
  {
    // alle Werte und Adressen löschen
    for (int i = 0; i < MAX_SENSORS; i++)
    {
      ds18b20Hub->_addresses[i] = 0;
      ds18b20Hub->_temperatures[i] = ds18b20Hub->DUMMY_TEMPERATURE;
    }
    // Anzahl der Sensoren am selben PIN (maximal 8) ermitteln
    vTaskDelay(10 / portTICK_PERIOD_MS);
    ds18b20Hub->_sensorCount = ds18x20_scan_devices(ds18b20Hub->_ioPin, ds18b20Hub->_addresses, MAX_SENSORS);
    if (ds18b20Hub->_sensorCount < 1)
    {
      printf("No sensors detected!\n");
    }
    else
    {
      // printf("%d sensors detected:\n", ds18b20Hub->_sensorCount);
      // auf MAX_SENSORS (derzeit 8) beschränken
      if (ds18b20Hub->_sensorCount > MAX_SENSORS)
      {
        ds18b20Hub->_sensorCount = MAX_SENSORS;
      }
      // Messung bei allen Sensoren starten
      for (int i = 0; i < ds18b20Hub->_sensorCount; i++)
      {
        ds18x20_addr_t addr = ds18b20Hub->_addresses[i];
        vTaskDelay(10 / portTICK_PERIOD_MS);
        esp_err_t err = ds18x20_measure(ds18b20Hub->_ioPin, addr, false);
        if (err != ESP_OK)
        {
          printf("!!!! ERROR, start measure(): %d\n", err);
        }
      }
      vTaskDelay(500 / portTICK_PERIOD_MS); // Warten, bis Sensor Werte ermittelt hat
      // Werte aus den Sensoren auslesen
      for (int i = 0; i < ds18b20Hub->_sensorCount; i++)
      {
        ds18x20_addr_t addr = ds18b20Hub->_addresses[i];
        float temp_c = 0.0;
        vTaskDelay(10 / portTICK_PERIOD_MS);
        esp_err_t err = ds18x20_read_temperature(ds18b20Hub->_ioPin, addr, &temp_c);
        if (err != ESP_OK)
        {
          printf("!!!! ERROR, measure(): %d\n", err);
        }
        else // gültiger Messwert ==> in Array abspeichern
        {
          uint32_t addr0 = addr >> 32;
          uint32_t addr1 = addr;
          char addrText[20];
          sprintf(addrText, "%08x%08x", addr0, addr1);
          // printf("  Sensor %s reports %f deg C \n", addrText, temp_c);
          ds18b20Hub->_temperatures[i] = temp_c;
        }
      }
      // // nicht benutzte Sensorplätze löschen
      // for (int j = ds18b20Hub->_sensorCount; j < MAX_SENSORS; j++)
      // {
      //   ds18b20Hub->_addresses[j] = 0;
      //   ds18b20Hub->_temperatures[j] = ds18b20Hub->DUMMY_TEMPERATURE;
      // }
    }
    // Wait for a little bit between each sample (note that the
    // ds18x20_measure_and_read_multi operation already takes at
    // least 750ms to run, so this is on top of that delay).
    vTaskDelay(LOOP_DELAY_MS / portTICK_PERIOD_MS);
  }
}

Ds18b20Hub::Ds18b20Hub(gpio_num_t ioPin)
{
  _ioPin = ioPin;
  xTaskCreate(ds18x20_test, "ds18x20_test", configMINIMAL_STACK_SIZE * 4, this, configMAX_PRIORITIES/2 , NULL);
}

float Ds18b20Hub::getTemperature(char *sensorAddress)
{
  for (int i = 0; i < _sensorCount; i++)
  {
    uint32_t addr0 = _addresses[i] >> 32;
    uint32_t addr1 = _addresses[i];
    char addrText[20];
    sprintf(addrText, "%08x%08x", addr0, addr1);
    // printf("getTemperature() addr: '%s' addrInArray: '%s'\n", sensorAddress, addrText);
    if (strcmp(addrText, sensorAddress) == 0)
    {
      // printf("getTemperature() return: %f\n", _temperatures[i]);
      return _temperatures[i];
    }
  }
  return DUMMY_TEMPERATURE;
}
