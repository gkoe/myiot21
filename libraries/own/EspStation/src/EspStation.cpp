#include "EspStation.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include <string.h>

#include <EspConfig.h>
#include <Logger.h>

// Event group
// static EventGroupHandle_t wifi_event_group;
#define ESP_MAXIMUM_RETRY 10
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
bool _isStationOnline = false;
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static int s_retry_num = 0;
// static const char *TAG = "wifi station";
static char _ipString[30];

static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        Logger.info("EspStation, eventHandler()", "SYSTEM_EVENT_STA_START");
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            Logger.error("EspStation, eventHandler()", "retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            Logger.error("EspStation, eventHandler()", "SYSTEM_EVENT_STA_DISCONNECTED !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        sprintf(_ipString, IPSTR,  IP2STR(&event->ip_info.ip));
        sprintf(loggerMessage, "EspStation, eventHandler() IP-ADDRESS %s ------------------------", _ipString);
        Logger.info("EspStation, init()", loggerMessage);
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void EspStationClass::init()
{
    // create the event group to handle wifi events
    s_wifi_event_group = xEventGroupCreate();

    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    char *ssid = EspConfig.getSsid();
    if (strlen(ssid) == 0)
    {
        Logger.error("EspStation, init()", "!!!!!!!!!!!!!!!!! SSID empty !!!!!!!!!!!!!!!!!");
    }
    char *password = EspConfig.getPassword();
    if (strlen(password) == 0)
    {
        Logger.error("EspStation, init()", "!!!!!!!!!!!!!!!!!!! PASSWORD empty !!!!!!!!!!!!!!!!!");
    }

    esp_netif_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // initialize the wifi stack in STAtion mode with config in RAM
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    // configure the wifi connection and start the interface
    wifi_sta_config_t staConfig = {}; // die beiden Klammern sind notwendig!
    memcpy(staConfig.ssid, ssid, strlen(ssid));
    memcpy(staConfig.password, password, strlen(password));
    staConfig.ssid[strlen(ssid)] = 0;
    staConfig.password[strlen(password)] = 0;
    wifi_config_t wifi_config = {
        .sta = staConfig};

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    sprintf(loggerMessage, "Connecting to %s\n", wifi_config.sta.ssid);
    Logger.info("EspStation, init()", loggerMessage);

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        sprintf(loggerMessage, "************** IP Address:  %s ******************", _ipString);
        _isStationOnline = true;
        Logger.info("EspStation, event_handler(), GotIp:", loggerMessage);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        sprintf(loggerMessage, "WIFI FAILED !!!!!!!!!!");
        _isStationOnline = false;
        Logger.info("EspStation, event_handler(),", loggerMessage);
    }
    else
    {
        sprintf(loggerMessage, "OTHER ERROR !!!!!!!!!!");
        _isStationOnline = false;
        Logger.info("EspStation, event_handler(),", loggerMessage);
    }

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler));
    vEventGroupDelete(s_wifi_event_group);
}

bool EspStationClass::isStationOnline()
{
    return _isStationOnline;
}


char* EspStationClass::getIpAddressString(){
    return _ipString;
}

EspStationClass EspStation;