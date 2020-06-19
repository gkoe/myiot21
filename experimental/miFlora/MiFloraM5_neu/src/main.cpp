#include <rom/rtc.h>
#include <M5Stack.h>
#include <WiFi.h>
#include "BleMiflora.h"
#include "MiFloraConfig.h"


const char* wifi_ssid     = "A1-B5035B";
const char* wifi_password = "52809766B6";

#define DEEP_SLEEP_DURATION_SECONDS 1800ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define MAX_WAKEUP_SECONDS 60         // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

#define MI_FLORAS 5

std::string macAddresses[MI_FLORAS] = { "C4:7C:8D:66:01:E6", "C4:7C:8D:65:58:87", "C4:7C:8D:64:3B:61", "C4:7C:8D:65:C0:7B", "C4:7C:8D:65:B5:D5" };
const char *names[MI_FLORAS] = {"agave", "oleander","bougainvillea", "strelizie", "sagopalme"};

RTC_DATA_ATTR int bootCount = 1;  // RTC-Daten überleben DeepSleep aber nicht einen echten Reset

/**
 * Setzt den Wake-Up-Timer und schickt den ESP32 in den DeepSleep
 * */
void goToDeepSleep(){
  esp_sleep_enable_timer_wakeup(DEEP_SLEEP_DURATION_SECONDS * 1000000ll);
  Serial.println("Going to sleep now.");
  esp_deep_sleep_start();
}

/**
 * Paralleler Task, der den ESP nach Ablauf der maximalen SLEEP_WAIT-Dauer
 * in den DeepSleep schickt
 * */
void taskDeepSleep( void * parameter ){
  delay(MAX_WAKEUP_SECONDS * 1000);
  goToDeepSleep();
}

/**
 * Gibt den Grund des letzten Neustarts aus
 * */
void verbose_print_reset_reason(RESET_REASON reason)
{
  switch ( reason)
  {
    case 1  : Serial.println ("Vbat power on reset");break;
    case 3  : Serial.println ("Software reset digital core");break;
    case 4  : Serial.println ("Legacy watch dog reset digital core");break;
    case 5  : Serial.println ("Deep Sleep reset digital core");break;
    case 6  : Serial.println ("Reset by SLC module, reset digital core");break;
    case 7  : Serial.println ("Timer Group0 Watch dog reset digital core");break;
    case 8  : Serial.println ("Timer Group1 Watch dog reset digital core");break;
    case 9  : Serial.println ("RTC Watch dog Reset digital core");break;
    case 10 : Serial.println ("Instrusion tested to reset CPU");break;
    case 11 : Serial.println ("Time Group reset CPU");break;
    case 12 : Serial.println ("Software reset CPU");break;
    case 13 : Serial.println ("RTC Watch dog Reset CPU");break;
    case 14 : Serial.println ("for APP CPU, reseted by PRO CPU");break;
    case 15 : Serial.println ("Reset when the vdd voltage is not stable");break;
    case 16 : Serial.println ("RTC Watch dog reset digital core and rtc module");break;
    default : Serial.println ("NO_MEAN");
  }
}

/**
 * Verbindet sich mit dem Wifi und dem Mqtt-Server, fragt den nächsten Miflora-Sensor über BLE ab
 * überträgt die Messdaten per Mqtt und legt sich wieder schlafen.
 */
void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Lcd.clear();
  M5.Lcd.setCursor(0,0);
  verbose_print_reset_reason(rtc_get_reset_reason(0));
  delay(1000);
  // Task registrieren, der ESP nach spätestens einer Minute schlafen schickt  
  xTaskCreate(      taskDeepSleep,          /* Task function. */
                    "TaskDeepSleep",        /* String with name of task. */
                    10000,                  /* Stack size in words. */
                    NULL,                   /* Parameter passed as input of the task */
                    1,                      /* Priority of the task. */
                    NULL);                  /* Task handle. */
  Serial.println(F("!MF: Starting Flora client..."));
  Serial.println(F("!MF: Connecting WiFi..."));
  WiFi.begin(wifi_ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Wifi connected
  Serial.println("");
  Serial.println(F("!MF: WiFi connected"));
  Serial.print(F("!MF: BootCount: "));
  Serial.println(bootCount);
  std::string macAddr = macAddresses[bootCount];
  const char *name = names[bootCount];
  Serial.print(F("!MF: Read from "));
  Serial.println(name);
  BleMiflora.init(macAddr, name);
  BleMiflora.getSensorDataAndPublishToMqtt();
  bootCount++;
  if(bootCount >= MI_FLORAS){
    bootCount=0;
  }
  delay(1000);
  Serial.println(F("!MF: end of transmission"));
  goToDeepSleep();
}

// Loop wird nie erreicht
void loop(){
  delay(5000);
}