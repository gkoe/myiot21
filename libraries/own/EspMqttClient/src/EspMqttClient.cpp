#include "EspMqttClient.h"
#include <EspConfig.h>
#include <Logger.h>
#include <Constants.h>
#include <HttpServer.h>
// #include <SystemService.h>

bool mqttIsConnected = false;
static esp_mqtt_client_handle_t client;

// const char *EspMqttClientClass::getMqttUserName() { return _mqttUserName; }
// const char *EspMqttClientClass::getMqttPassword() { return _mqttPassword; }
// const char *EspMqttClientClass::getLastWillTopic() { return _lastWillTopic; }
const char *EspMqttClientClass::getMainTopic() { return _mainTopic; }
const char *EspMqttClientClass::getMqttThingName() { return _thingName; }
const char *EspMqttClientClass::getMqttServer() { return _mqttBroker; }
int EspMqttClientClass::getMqttPort() { return _mqttPort; }

/**
 * Zentraler Eventhandler für alle Mqtt-Events. 
 * Connect/Disconnect wird über die Events gesetzt
 * client wird ebenfalls über die empfangenen Events gespeichert
 * Empfangene Messages werden an die Abonennten weitergeleitet
 */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event)
{
  client = event->client;
  int msg_id;
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  char topic[LENGTH_TOPIC];
  char payload[LENGTH_PAYLOAD];
  int mainTopicLength = 0;
  int restTopicLength = 0;
  // int topicLen;
  // int payloadLen;

  // your_context_t *context = event->context;
  switch (event->event_id)
  {
  case MQTT_EVENT_BEFORE_CONNECT:
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "mqtt_event_handler(),  MQTT_EVENT_BEFORE_CONNECT");
    Logger.info("EspMqttClient, mqtt_event_handler()", loggerMessage);
    break;
  case MQTT_EVENT_CONNECTED:
    char registeredTopic[LENGTH_TOPIC];
    sprintf(registeredTopic, "%s/#", EspMqttClient.getMainTopic());
    Logger.info("EspMqttClient, mqtt_event_handler()", "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, registeredTopic, 0);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "%s, Result: %d", registeredTopic, msg_id);
    Logger.info("EspMqttClient, mqtt_event_handler, Topic subscribed", loggerMessage);
    mqttIsConnected = true;
    break;
  case MQTT_EVENT_DISCONNECTED:
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "mqtt_event_handler(),  MQTT_EVENT_DISCONNECTED");
    Logger.info("EspMqttClient, mqtt_event_handler()", loggerMessage);
    mqttIsConnected = false;
    break;
  case MQTT_EVENT_SUBSCRIBED:
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "mqtt_event_handler(),  MQTT_EVENT_SUBSCRIBED");
    Logger.info("EspMqttClient, mqtt_event_handler()", loggerMessage);
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    Logger.info("EspMqttClient, mqtt_event_handler()", "MQTT_EVENT_UNSUBSCRIBED");
    break;
  case MQTT_EVENT_PUBLISHED:
    Logger.info("EspMqttClient, mqtt_event_handler()", "MQTT_EVENT_PUBLISHED");
    break;
  case MQTT_EVENT_DATA:
    mainTopicLength = strlen(EspMqttClient.getMainTopic());
    restTopicLength = event->topic_len - mainTopicLength - 1; // wegen / hinter maintopic
    strncpy(topic, event->topic + mainTopicLength + 1, restTopicLength);
    strncpy(payload, event->data, event->data_len);
    topic[restTopicLength] = 0;
    payload[event->data_len] = 0;
    // printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    // printf("DATA=%.*s\r\n", event->data_len, event->data);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "mqtt_event_handler(),  topicLength: %d, dataLength: %d", event->topic_len, event->data_len);
    Logger.info("EspMqttClient, length of data", loggerMessage);
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "mqtt_event_handler(),  MQTT_EVENT_DATA, topic: %s, data: %s", topic, payload);
    Logger.info("EspMqttClient, mqtt_event_handler()", loggerMessage);
    EspMqttClient.notifySubscribers(topic, payload);
    break;
  case MQTT_EVENT_ERROR:
    Logger.error("EspMqttClient, mqtt_event_handler()", "MQTT_EVENT_ERROR");
    //SystemService.pushError("EspMqttClient, publish-error");
    break;
  }
  return ESP_OK;
}

/**
 * Beginnt der Text mit den Zeichen des Patterns?
 */
bool strStartsWith(char *text, char *pattern)
{
  int length = strlen(pattern);
  if (strlen(text) < length)
  {
    return false;
  }
  for (int i = 0; i < length; i++)
  {
    if (text[i] != pattern[i])
      return false;
  }
  return true;
}

/**
 * Überprüft, welche Subscriber das Topic registriert haben und 
 * verständigt die entsprechenden Subscriber.
 * //! Derzeit wird nur überprüft ob das empfangene Topic mit dem registrierten
 *      Text beginnt. Später auch um Wildcards erweitern (Readme.md auch updaten).
 */
void EspMqttClientClass::notifySubscribers(char *topic, char *payload)
{
  Logger.info("MqttClient notify Subscribers, received topic: ", topic);
  for (std::list<MqttSubscription *>::iterator it = _mqttSubscriptions.begin(); it != _mqttSubscriptions.end(); it++)
  {
    MqttSubscription *subscriptionPtr = *it;
    Logger.info("MqttClient notify Subscribers, subscribed topic: ", subscriptionPtr->topic);
    if (strStartsWith(topic, subscriptionPtr->topic))
    {
      subscriptionPtr->subscriberCallback(topic, payload);
    }
  }
}

// MqttCallback* EspMqttClientClass::getSubscriberCallback(){
//   return _subscriberCallback;
// }

/**
 * Eine Komponente (meist Actor) kann sich über eine Subscription als Subscriber 
 * registrieren. Das Topic wird dann um das root-Topic erweitert. Der zentrale Subscriber selektiert
 * auf Basis des übergebenen Topics empfangene Nachrichten aus und schickt sie weiter.
 */
void EspMqttClientClass::addSubscription(MqttSubscription *subscriptionPtr)
{
  char subscriptionTopic[LENGTH_TOPIC];
  snprintf(subscriptionTopic, LENGTH_TOPIC - 1, "%s/command", subscriptionPtr->topic);
  strcpy(subscriptionPtr->topic, subscriptionTopic);
  _mqttSubscriptions.push_back(subscriptionPtr);
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  sprintf(loggerMessage, "Subscription: %s", subscriptionPtr->topic);
  Logger.info("MqttClient add Subscription", loggerMessage);
}

/**
 * eigene http-Route zum Test, ob eine MQTT-Verbindung besteht
 */
static esp_err_t testmqttrequestHandler(httpd_req_t *req)
{
  Logger.info("EspMqttClient, handleTestConnectionRequest()", "Start");
  char topic[LENGTH_TOPIC];
  char message[LENGTH_SHORT_TEXT] = "Connectiontest";
  char myprivate_response[300];

  const char *thingName = EspConfig.getThingName();
  sprintf(topic, "%s/mqttconnectiontest", thingName);
  bool ok = EspMqttClient.publish(topic, message);
  if (ok)
  {
    const char *mqttBroker = EspConfig.getMqttBroker();
    sprintf(myprivate_response, "Mqtt-Message sent, subscribe %s on mqtt-broker %s", topic, mqttBroker);
  }
  else
  {
    sprintf(myprivate_response, "Connection to mqttbroker lost!!!");
  }
  httpd_resp_send(req, myprivate_response, strlen(myprivate_response));
  return ESP_OK;
}

static const httpd_uri_t testmqttrequest = {
    .uri = "/testmqttrequest",
    .method = HTTP_GET,
    .handler = testmqttrequestHandler,
    .user_ctx = nullptr};

/**
 * Verbindung mit dem MQTT-Broker initialisieren.
 * mainTopic wird als root-Topic für publish und subscribe verwendet
 */
void EspMqttClientClass::init(const char *mainTopic)
{
  Logger.info("EspMqttClient, init(), maintopic", mainTopic);
  strcpy(_mainTopic, mainTopic);
  _mqttBroker = EspConfig.getMqttBroker();
  _mqttPort = EspConfig.getMqttBrokerPort();
  _thingName = EspConfig.getThingName();
  EspConfig.getNvsStringValue("user", _mqttUserName);
  EspConfig.getNvsStringValue("password", _mqttPassword);
  EspConfig.getNvsStringValue("lastwill", _lastWillTopic);
  if (strcmp(_lastWillTopic, "true") == 0)
  {
    sprintf(_lastWillTopic, "%s/thing/lastwill", _thingName);
  }
  else
  {
    strcpy(_lastWillTopic, "");
  }
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MQTT-Broker Address: %s:%i", _mqttBroker, _mqttPort);
  char uri[LENGTH_MIDDLE_TEXT];
  sprintf(uri, "mqtt://%s:%d", _mqttBroker, _mqttPort);
  esp_mqtt_client_config_t mqtt_cfg = {};
  mqtt_cfg.uri = uri;
  mqtt_cfg.event_handle = mqtt_event_handler;
  snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MQTT-URI: %s", uri);
  Logger.info("EspMqttClient, init()", loggerMessage);
  snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MqttConfig-URI: %s", mqtt_cfg.uri);
  Logger.info("EspMqttClient, init()", loggerMessage);

  if (strlen(uri) < 10)
  {
    snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "MQTT not configured");
    Logger.error("EspMqttClient, init()", loggerMessage);
    return;
  }

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  int result = esp_mqtt_client_start(client);
  snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "ResultCode: %d", result);
  Logger.info("EspMqttClient, init(), startClient", loggerMessage);
  HttpServer.addRoute(&testmqttrequest);
}

/**
 * Für das Topic wird die payload an den Broker übertragen
 */
bool EspMqttClientClass::publish(const char *topic, const char *payload)
{
  if (!mqttIsConnected)
  {
    Logger.error("MqttClient, publish", "Client not connected");
    return false;
  }
  char loggerMessage[LENGTH_LOGGER_MESSAGE];
  Logger.info("MqttClient, publish", "Start");
  // publish with retained-flag
  char totalTopic[LENGTH_TOPIC];
  sprintf(totalTopic, "%s/%s", _mainTopic, topic);
  int result = -1;
  result = esp_mqtt_client_publish(client, totalTopic, payload, 0, 0, 1);
  snprintf(loggerMessage, LENGTH_LOGGER_MESSAGE - 1, "Topic: %s , Payload: %s published, result: %d", totalTopic, payload, result);
  Logger.info("MqttClient publish", loggerMessage);
  return true;
}

EspMqttClientClass EspMqttClient;
