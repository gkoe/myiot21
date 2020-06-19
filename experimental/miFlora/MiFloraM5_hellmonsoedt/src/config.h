// #define FLORA_ADDR "C4:7C:8D:66:01:E6"

// #define MQTT_TEMPERATURE MQTT_DEV_BASE "temperature"
// #define MQTT_MOISTURE MQTT_DEV_BASE "moisture"
// #define MQTT_LIGHT MQTT_DEV_BASE "light"
// #define MQTT_CONDUCTIVITY MQTT_DEV_BASE "conductivity"
// #define MQTT_BATTERY MQTT_DEV_BASE "battery"

#define MI_FLORAS 7


std::string macAddresses[MI_FLORAS] = { "C4:7C:8D:65:58:87", "C4:7C:8D:65:C0:7B", "C4:7C:8D:64:3B:61", 
                                        "C4:7C:8D:67:69:A6", "C4:7C:8D:67:6A:43", "C4:7C:8D:67:2E:5C", "C4:7C:8D:65:B5:D5"};
const char *names[MI_FLORAS] = {"oleander","strelizie", "sagopalme", "Beaugainville", "Beaucarnea", "Orangenbaum", "strelizie2"};

#define SLEEP_DURATION  360ll // duration of sleep between flora connection attempts in seconds (must be constant with "ll" suffix)
#define SLEEP_WAIT 60         // time until esp32 is put into deep sleep mode. must be sufficient to connect to wlan, connect to xiaomi flora device & push measurement data to MQTT

#define BATTERY_INTERVAL 1   // Übertragung des Batteriezustandes alle xx Aufweckphasen

const char* wifi_ssid     = "x282629";
const char* wifi_password = "novaKla87Sie57";

const char* mqtt_server = "leonding.synology.me";
const int port = 48888;
