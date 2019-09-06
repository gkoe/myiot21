/* HTTPS GET Example using plain mbedTLS sockets
 *
 * Contacts the howsmyssl.com API via TLS v1.2 and reads a JSON
 * response.
 *
 * Adapted from the ssl_client1 example in mbedtls.
 *
 * Original Copyright (C) 2006-2016, ARM Limited, All Rights Reserved, Apache 2.0 License.
 * Additions Copyright (C) Copyright 2015-2016 Espressif Systems (Shanghai) PTE LTD, Apache 2.0 License.
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"

#include <Logger.h>
#include <LoggerTarget.h>
#include <SerialLoggerTarget.h>
#include <EspStation.h>
#include <HttpServer.h>
#include <EspConfig.h>
#include <EspTime.h>
#include <EspMqttClient.h>
#include <SystemService.h>
#include <EspUdp.h>
#include <UdpLoggerTarget.h>

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "leonding.synology.me"
#define WEB_PORT "443"
#define WEB_URL "https://leonding.synology.me/esplogs"

static const char *TAG = "example";

// LetsEncryptRoot.cer
const char *root_ca =
    "-----BEGIN CERTIFICATE-----\n\
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/\
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow\
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD\
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB\
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O\
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq\
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b\
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw\
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD\
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV\
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG\
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69\
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr\
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz\
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5\
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo\
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ\
\n-----END CERTIFICATE-----\n";

const char *after_root_ca = "";

extern "C"
{
    void app_main(void);
}

/* Root cert for howsmyssl.com, taken from server_root_cert.pem
   The PEM file was extracted from the output of this command:
   openssl s_client -showcerts -connect www.howsmyssl.com:443 </dev/null
   The CA root cert is the last cert given in the chain of certs.
   To embed it in the app binary, the PEM file is named
   in the component.mk COMPONENT_EMBED_TXTFILES variable.
*/
// extern const uint8_t server_root_cert_pem_start[] asm("_binary_server_root_cert_pem_start");
// extern const uint8_t server_root_cert_pem_end[]   asm("_binary_server_root_cert_pem_end");

static void https_get_task(void *pvParameters)
{
    char buf[512];
    int ret, len;

    while (1)
    {
        esp_tls_cfg_t cfg = {};
        cfg.cacert_pem_buf = (const unsigned char *)root_ca;
        cfg.cacert_pem_bytes = strlen(root_ca);

        struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);
        char request[500];
        sprintf(request, "GET %s HTTP/1.0\r\nHost: %s \r\nUser-Agent: esp-idf/1.0 esp32\r\n\r\n", WEB_URL, WEB_SERVER);

        if (tls != NULL)
        {
            ESP_LOGI(TAG, "Connection established...");
            size_t written_bytes = 0;
            do
            {

                ret = esp_tls_conn_write(tls,
                                         request + written_bytes,
                                         strlen(request) - written_bytes);
                if (ret >= 0)
                {
                    ESP_LOGI(TAG, "%d bytes written", ret);
                    written_bytes += ret;
                }
                else if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE)
                {
                    ESP_LOGE(TAG, "esp_tls_conn_write  returned 0x%x", ret);
                    break;
                }
            } while (written_bytes < strlen(request));

            ESP_LOGI(TAG, "Reading HTTP response...");

            do
            {
                len = sizeof(buf) - 1;
                bzero(buf, sizeof(buf));
                ret = esp_tls_conn_read(tls, (char *)buf, len);

                if (ret == MBEDTLS_ERR_SSL_WANT_WRITE || ret == MBEDTLS_ERR_SSL_WANT_READ)
                    continue;

                if (ret < 0)
                {
                    ESP_LOGE(TAG, "esp_tls_conn_read  returned -0x%x", -ret);
                    break;
                }

                if (ret == 0)
                {
                    ESP_LOGI(TAG, "connection closed");
                    break;
                }

                len = ret;
                ESP_LOGD(TAG, "%d bytes read", len);
                /* Print response directly to stdout as it is read */
                for (int i = 0; i < len; i++)
                {
                    putchar(buf[i]);
                }
            } while (1);
        }
        else
        {
            ESP_LOGE(TAG, "Connection failed...");
        }

        esp_tls_conn_delete(tls);
        putchar('\n'); // JSON output doesn't have a newline at end

        static int request_count;
        ESP_LOGI(TAG, "Completed %d requests", ++request_count);

        for (int countdown = 10; countdown >= 0; countdown--)
        {
            ESP_LOGI(TAG, "%d...", countdown);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        ESP_LOGI(TAG, "Starting again!");
    }
}

void app_main(void)
{
    printf("==============\n");
    printf("UdpLogger Test\n");
    printf("==============\n");
    EspConfig.init();
    const char *thingName = EspConfig.getThingName();
    Logger.init("SslTest");
    SerialLoggerTarget *serialLoggerTarget = new SerialLoggerTarget("SLT", LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(serialLoggerTarget);
    SystemService.init();
    EspStation.init();
    Logger.info("SslTest, app_main()", "Waiting for connection!");
    while (!EspStation.isStationOnline())
    {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    HttpServer.init();
    Logger.info("SslTest, app_main()", "HttpServer started");
    EspTime.init();
    EspUdp.init();
    UdpLoggerTarget *udpLoggerTargetPtr = new UdpLoggerTarget("ULT", LOG_LEVEL_VERBOSE);
    Logger.addLoggerTarget(udpLoggerTargetPtr);

    char loggerMessage[LENGTH_LOGGER_MESSAGE];
    EspConfig.getConfig(loggerMessage, LENGTH_LOGGER_MESSAGE - 1);
    Logger.info("SslTest, Config", loggerMessage);
    Logger.info("SslTest, app_main()", "SslTest running!");

    xTaskCreate(&https_get_task, "https_get_task", 8192, NULL, 5, NULL);

    while (true)
    {
        SystemService.checkSystem();
        vTaskDelay(1);
    }
}
