# MiFloraM5

![UI](images/miflora.png)

## Ziele
Die preisgünstigen Bluetooth-BLE Pflanzensensoren MiFlora von Xiaomi sollen in das Homeautomation-System über MQTT eingebunden werden.
Diese Sensoren werden mit einer 2032-Knopfzelle versorgt. Daher ist ein sparsamer Umgang mit den Abfragen der Messwerte unbedingt erforderlich. Der ESP32 verfügt über eine Bluetooth-BLE-Schnittstelle und eignet sich daher sehr gut als Gateway zwischen den Sensoren und dem IP-gestützten Homeautomation-Netzwerk.

## Lösungsideen
Der ESP32 wird im Deep-Sleep-Modus betrieben, wacht regelmäßig (z.B. alle Stunden) auf, fragt einen Sensor ab und überträgt die empfangenen Daten (falls die Abfrage geklappt hat) an den Mqtt-Server. Bei z.B. sechs Sensoren werden damit pro Tag vier Messwerte erfasst, was als Gießhilfe leicht ausreicht. Die Sensoren und deren Mac-Adressen sind im Prototyp fix im Quellcode abgespeichert. Der Index des aktuell abgefragten Sensors wird im RTC-Memory abgelegt (dessen Inhalt überlebt einen DeepSleep).

Neben der Bodenfeuchtigkeit wird auch die Helligkeit und die Uhrzeit der Messung übertragen. Sollte ein Sensor nicht per BLE erreichbar sein, kommt er im nächsten Zyklus wieder dran. Das Homeautomation-System kann bei länger andauernden Problemen mit der Erreichbarkeit den Benutzer benachrichtigen. 

Der Betrieb mit DeepSleep erlaubt die optimale Positionierung des ESP32-BLE-Gateways in der Mitte der MiFlora-Sensoren durch den autarken Betrieb über eine solargespeiste Powerbank.

## Abhängigkeiten

### BLE-Library
Die von Neil Kolban erstellte BLE-Library (https://github.com/nkolban/ESP32_BLE_Arduino ) ist sehr umfangreich und sprengt die verfügbaren Ressourcen des ESP32. Das Programm ist aber trotzdem lauffähig.

### PubSubLibrary zur Kommunikation mit dem MQTT-Broker

### Verwendung der DeepSleep-Fähigkeiten des ESP32
Im Vergleich zum ESP8266 bietet der ESP32 viel Mehr Möglichkeiten im Zusammenhang mit DeepSleep (unterschiedliche WakeUp-Sources, größerer DeepSleep-Zeitraum, ...)


## ToDos
* Der Speicher des ESP32 ist so zu partionieren, dass beim Linken keine Fehlermeldungen anfallen
* Einbindung des WiFi-Managers und des Config-Systems



