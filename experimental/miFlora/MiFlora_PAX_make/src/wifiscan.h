#ifndef _WIFISCAN_H
#define _WIFISCAN_H

// ESP32 Functions
#include <Arduino.h>
#include <esp_wifi.h>
//#include "globals.h"

// Hash function for scrambling MAC addresses
//#include "hash.h"

void wifi_sniffer_init(void);
void IRAM_ATTR wifi_sniffer_packet_handler(void *buff, wifi_promiscuous_pkt_type_t type);
void switchWifiChannel(TimerHandle_t xTimer);

#endif