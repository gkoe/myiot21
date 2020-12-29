#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"

#include <EspAp.h>

extern "C" {
	void app_main(void);
}

void app_main()
{
  printf("====================\n");
  printf("Esp Accesspoint Test\n");
  printf("====================\n");
  EspAp.init();

  while (!EspAp.isApStarted())
  {
    printf("Waiting for start ...\n");
    vTaskDelay(1000 / portTICK_RATE_MS);
  }
  printf("*********** Accesspoint started! *******************\n");
  
}