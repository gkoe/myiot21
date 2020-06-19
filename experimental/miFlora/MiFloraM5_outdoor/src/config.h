// #define FLORA_ADDR "C4:7C:8D:66:01:E6"

// #define MQTT_TEMPERATURE MQTT_DEV_BASE "temperature"
// #define MQTT_MOISTURE MQTT_DEV_BASE "moisture"
// #define MQTT_LIGHT MQTT_DEV_BASE "light"
// #define MQTT_CONDUCTIVITY MQTT_DEV_BASE "conductivity"
// #define MQTT_BATTERY MQTT_DEV_BASE "battery"

#define MI_FLORAS 5


std::string macAddresses[MI_FLORAS] = { "C4:7C:8D:66:01:E6", "C4:7C:8D:65:58:87", "C4:7C:8D:64:3B:61", "C4:7C:8D:65:C0:7B", "C4:7C:8D:65:B5:D5" };
const char *names[MI_FLORAS] = {"agave", "oleander","bougainvillea", "strelizie", "sagopalme"};

#define SLEEP_DURATION  7200ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 60         // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

#define BATTERY_INTERVAL 10   // Übertragung des Batteriezustandes alle xx Aufweckphasen

const char* wifi_ssid     = "A1-B5035B";
const char* wifi_password = "52809766B6";

const char* mqtt_server = "10.0.0.125";
