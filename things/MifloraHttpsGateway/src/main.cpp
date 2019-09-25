#include <Arduino.h>

#include "Logger.h"
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <HttpClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>

const char *SERIAL_LOGGER_TAG = "SLT";
#define SLEEP_DURATION 720ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 90		 // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

//>>>>>>>>>>>>>>>>>>>> Thingspezifisch
#include <MiFloraHttps.h>
//<<<<<<<<<<<<<<<<<<<<<<<

void gotoDeepSleep(void *parameter)
{
	delay(SLEEP_WAIT * 1000);
	esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ll);
	Logger.info("MiFloraGateway;goToDeepSleep()", "Going to sleep long now.");
	esp_deep_sleep_start();
}

void setup()
{
	printf("===============\n");
	printf("Miflora-Gateway\n");
	printf("===============\n");
	EspConfig.init();
	Logger.init("MiFloraGateway");
	SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget(SERIAL_LOGGER_TAG, LOG_LEVEL_VERBOSE);
	Logger.addLoggerTarget(serialLoggerTarget);
	SystemService.init();
	SystemService.heapSizeCanPushError(false); // BLE braucht viel zu viel Speicher
	SystemService.resetRestartsCounter();
	EspStation.init();
	Logger.info("MiFloraGateway, app_main()", "Waiting for connection!");
	while (!EspStation.isStationOnline())
	{
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
	HttpServer.init();
	Logger.info("MiFloraGateway, app_main()", "HttpServer started");
	EspTime.init();
	EspUdp.init();
	UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
	Logger.addLoggerTarget(udpLoggerTargetPtr);
	
	xTaskCreate(gotoDeepSleep,   /* Task function. */
				"gotoDeepSleep", /* String with name of task. */
				2048,			 /* Stack size in words. */
				NULL,			 /* Parameter passed as input of the task */
				1,				 /* Priority of the task. */
				NULL);			 /* Task handle. */

	MiFloraHttps.init();
	Logger.info("MiFloraGateway, app_main()", "After MiFlora.init()");
	Logger.info("MiFloraGateway, app_main()", "Waiting for deepsleep!");

}

void loop()
{
	SystemService.checkSystem();
	vTaskDelay(1);
}