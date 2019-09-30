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

#define LED_BUILTIN_PIN 16 // WEMOS MINI32 2, TTGO 16, Lolin 5, sonst 21

const char *SERIAL_LOGGER_TAG = "SLT";
#define SLEEP_DURATION 720ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 90		 // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

const char *urlMifloras = "leonding.synology.me/esplogs/mqtt"; //mifloraentries
const char *basicAuthenticationName = "gerald";
const char *basicAuthenticationPassword = "piKla87Sie57";


#include <MiFlora.h>

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

/**
 * Messwert per https an den Server übertragen
 */
void sendByHttps(const char *mac, const char *sensorName, const char *value)
{
	char topic[LENGTH_TOPIC];
	char payload[LENGTH_PAYLOAD];
	char body[LENGTH_LOGGER_MESSAGE];

	sprintf(topic, "\"miflora/%s/%s/state\"", mac, sensorName);
	sprintf(payload, "{\"timestamp\": %ld,\"value\": %s}", EspTime.getTime(), value);
	sprintf(body, "{\"topic\": %s,\"payload\": %s}", topic, payload);
	// sprintf(body, "{\"mac\": \"%s\",\"moisture\": %s,\"temperature\": %s,\"brightness\": %s,\"batteryLevel\": %s}",
	// 		mac, "99", "33", "9999", "99");
	Logger.debug("MiFloraGateway, send by https: ", body);
	HttpClient.post(urlMifloras, body, true, "gerald", "piKla87Sie57");
}

void setup()
{
	// char loggerMessage[LENGTH_LOGGER_MESSAGE];
	printf("===============\n");
	printf("Miflora-Gateway\n");
	printf("===============\n");
	EspConfig.init();
	// const char *thingName = EspConfig.getThingName();
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
	if (strlen(mac) > 0)  // Es sind MiFlora-Daten im NVS gespeichert und per Mqtt oder http zu übertragen
	{
		xTaskCreate(taskDeepSleepLong,   /* Task function. */
					"TaskDeepSleepLong", /* String with name of task. */
					2048,				 /* Stack size in words. */
					NULL,				 /* Parameter passed as input of the task */
					1,					 /* Priority of the task. */
					NULL);				 /* Task handle. */
		// char payload[LENGTH_LOGGER_MESSAGE];
		char moisture[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("moisture", moisture);
		char temperature[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("temperature", temperature);
		char brightness[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("brightness", brightness);
		char batteryLevel[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("batteryLevel", batteryLevel);
		char rssi[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("rssi", rssi);
		char conductivity[LENGTH_SHORT_TEXT];
		EspConfig.getNvsStringValue("conductivity", conductivity);
		printf("Batterylevel: '%s'\n", batteryLevel);
		if (strlen(batteryLevel) == 0)
		{
			sendByHttps(mac, "batteryLevel", batteryLevel);
			// strcpy(batteryLevel, "-1.0");
		}
		sendByHttps(mac, "moisture", moisture);
		sendByHttps(mac, "temperature", temperature);
		sendByHttps(mac, "brightness", brightness);
		sendByHttps(mac, "rssi", rssi);
		sendByHttps(mac, "conductivity", conductivity);

		EspConfig.setNvsStringValue("mac", "");
	}
	else // keine Messungen anstehend ==> MiFlora-Scanmode ==> BLE
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
	vTaskDelay(1);
}