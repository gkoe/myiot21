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
#include <Thing.h>

#define LED_BUILTIN_PIN 16 // WEMOS MINI32 2, TTGO 16, Lolin 5, sonst 21

const char *SERIAL_LOGGER_TAG = "SLT";
#define SLEEP_DURATION 720ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 90		 // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

const char *urlMifloras = "leonding.synology.me/esplogs/mifloraentries";

//>>>>>>>>>>>>>>>>>>>> Thingspezifisch
#include <MiFlora.h>
//<<<<<<<<<<<<<<<<<<<<<<<

void taskDeepSleepShort(void *parameter)
{
	delay(SLEEP_WAIT * 1000);
	esp_sleep_enable_timer_wakeup(1000ll);
	Logger.info("MiFloraGateway;goToDeepSleep()", "Going to sleep short now.");
	esp_deep_sleep_start();
}

void taskDeepSleepLong(void *parameter)
{
	delay(SLEEP_WAIT * 1000);
	esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ll);
	Logger.info("MiFloraGateway;goToDeepSleep()", "Going to sleep long now.");
	esp_deep_sleep_start();
}

void setup()
{
	char loggerMessage[LENGTH_LOGGER_MESSAGE];
	printf("==========================\n");
	printf("Thing with Miflora-Gateway\n");
	printf("==========================\n");
	EspConfig.init();
	const char *thingName = EspConfig.getThingName();
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

	Logger.info("MiFloraGateway, app_main()", "Thing created");
	char mac[LENGTH_SHORT_TEXT];
	EspConfig.getNvsStringValue("mac", mac);
	// Logger.info("MiFloraGateway, app_main(), NvsTopicText=", mifloraTopics);
	if (strlen(mac) > 0)
	{
		xTaskCreate(taskDeepSleepLong,   /* Task function. */
					"TaskDeepSleepLong", /* String with name of task. */
					2048,				 /* Stack size in words. */
					NULL,				 /* Parameter passed as input of the task */
					1,					 /* Priority of the task. */
					NULL);				 /* Task handle. */
		char payload[LENGTH_LOGGER_MESSAGE];
		char moisture[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("moisture", moisture);
		char temperature[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("temperature", temperature);
		char brightness[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("brightness", brightness);
		char batteryLevel[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("batteryLevel", batteryLevel);
		printf("Batterylevel: '%s'\n", batteryLevel);
		if(strlen(batteryLevel)==0){
			strcpy(batteryLevel, "-1.0");
		}

		sprintf(payload, "{\"mac\": \"%s\",\"moisture\": %s,\"temperature\": %s,\"brightness\": %s,\"batteryLevel\": %s}",
		      mac, moisture, temperature, brightness, batteryLevel);
		Logger.debug("MiFloraGateway, send by https:", payload);
		HttpClient.post(urlMifloras, payload, true, "gerald", "piKla87Sie57");

		EspConfig.setNvsStringValue("mac", "");
	}
	else // keine Messungen anstehend ==> MiFlora-Mode ==> BLE
	{
		Logger.info("MiFloraGateway, app_main()", "Measure Miflora per BLE");
		xTaskCreate(taskDeepSleepShort,   /* Task function. */
					"TaskDeepSleepShort", /* String with name of task. */
					2048,				  /* Stack size in words. */
					NULL,				  /* Parameter passed as input of the task */
					1,					  /* Priority of the task. */
					NULL);				  /* Task handle. */
		MiFlora.init();
		MiFlora.readNextMiFlora();
	}
}

void loop()
{
	SystemService.checkSystem();
	Thing.refreshSensorsAndActors();
	vTaskDelay(1);
}