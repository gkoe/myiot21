#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <Constants.h>
#include <EspAp.h>

// #define EXAMPLE_ESP_WIFI_SSID      "MyAp"
#define EXAMPLE_ESP_WIFI_PASS      "my-Iot-21"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       3

static const char *TAG = "wifi softAP";
static const char *apPassword = "my-Iot-21";
bool __isApStarted = false;

EspApClass::EspApClass()
{
}

void getSsidText(char ssidText[])
{
    uint8_t mac[8];
    esp_err_t ok = esp_efuse_mac_get_default(mac);
    if (ok != ESP_OK)
    {
        sprintf(ssidText, "MAC_ERR_%d", ok);
        return;
    }
    sprintf(ssidText, "ESP_%02X%02X%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);
}


static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));

    char ssidText[32];
    for (int i = 0; i < 32; i++)
        ssidText[i] = 0;
    getSsidText(ssidText);
    printf("SSID: %s, Len: %d\n", ssidText, strlen(ssidText));
    wifi_ap_config_t wifi_ap_config;
    memcpy(wifi_ap_config.ssid, ssidText, strlen(ssidText));
    wifi_ap_config.ssid_len = strlen(ssidText);
    int len = strlen(apPassword);
    for(int i = 0; i<len; i++){
        wifi_ap_config.password[i] = apPassword[i];
    }
    wifi_ap_config.password[len] = 0;
    memcpy(wifi_ap_config.password, apPassword, 9);
    wifi_ap_config.channel = 0;
    wifi_ap_config.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_ap_config.beacon_interval = 100;
    wifi_ap_config.max_connection = 4;
    // wifi_ap_config.ssid_hidden = 0;

    wifi_config_t wifi_config = {
        .ap = wifi_ap_config
    };

    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             ssidText, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
    __isApStarted = true;
}


// print the list of connected stations
void printStationList()
{
    printf("\n");
    printf(" Connected stations:\n");
    printf("--------------------------------------------------\n");

    wifi_sta_list_t wifi_sta_list;
    tcpip_adapter_sta_list_t adapter_sta_list;

    // memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
    // memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));

    ESP_ERROR_CHECK(esp_wifi_ap_get_sta_list(&wifi_sta_list));
    ESP_ERROR_CHECK(tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list));

    for (int i = 0; i < adapter_sta_list.num; i++)
    {

        tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];
     	char ipText[100];

        esp_ip4addr_ntoa(&station.ip, ipText, 100-1);
        printf("%d - mac: %.2x:%.2x:%.2x:%.2x:%.2x:%.2x - IP: %s\n", i + 1,
               station.mac[0], station.mac[1], station.mac[2],
               station.mac[3], station.mac[4], station.mac[5],
               ipText);
    }

    printf("\n");
}


bool EspApClass::isApStarted(){
    return __isApStarted;
}

// Station list task, print station list every 10 seconds
void station_list_task(void *pvParameter)
{
    while (1)
    {

        printStationList();
        vTaskDelay(10000 / portTICK_RATE_MS);
    }
}



void EspApClass::init()
{

       //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // start the report stationlist task
    xTaskCreate(&station_list_task, "station_list_task", 2048, NULL, 5, NULL);
}

EspApClass EspAp;