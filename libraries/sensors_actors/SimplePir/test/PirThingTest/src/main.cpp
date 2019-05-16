#include <Arduino.h>

#define LED_BUILTIN_PIN 2 // LOLIN D32

#include <Arduino.h>
#include <HttpServer.h>
#include <ThingTime.h>
#include <ThingConfig.h>
#include <MqttClient.h>
#include <Logger.h>
#include <IotSensor.h>
#include <IotActor.h>
#include <Thing.h>
#include <SerialLoggerTarget.h>
#include <Logger.h>

//>>>>>>>>>>>>>>>>>>>> Thingspezifisch
#include <SimplePir.h>

#define PIR_PIN 26
//<<<<<<<<<<<<<<<<<<<<<<<

void setup() {
	Serial.begin(115200);                 //Initialisierung der seriellen Schnittstelle
	Serial.println();
	Serial.println();
	Serial.println(F("*TT ThingTest fuer PIR"));
	Serial.println(F("*TT =================="));
	ThingConfig.readConfig();			// Einlesen der Konfiguration in den JSON-Cache
	HttpServer.init();					// HttpServer initialisieren
	ThingTime.setNtpTimeSubscriber();   // Zeit über Internet synchronisieren	
	const char* thingName = ThingConfig.getValue("thingname");
	Thing.init(thingName, true);		// Thing initialisieren	mit JSON als Messageformat     
	
	// >>>>>>>>>>>>>>>>>>>>>>  Thingspezifischer Teil
  	IotSensor* pirPtr = new SimplePir(PIR_PIN,30,thingName,"PIR","",0);
  	Thing.addSensor(pirPtr);
	//<<<<<<<<<<<<<<<<<<<<<<< Ende Thingspezifischer Teil
	SerialLoggerTarget* loggerTarget = new SerialLoggerTarget("udplogger", 0);
  	Logger.addLoggerTarget(loggerTarget);
	MqttClient.subscribeToBroker();
}

void loop() {
	HttpServer.handleClient();
	MqttClient.doLoop();					// Mqtt-Schnittstelle bedienen
	Thing.refreshSensorsAndActors();
	delay(1);
}
