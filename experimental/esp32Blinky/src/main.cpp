#include <Arduino.h>
#define LED_BUILTIN 2

void setup()
{
    Serial.begin(115200);
    Serial.println("Hello");
    delay(200);
    pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
    digitalWrite(LED_BUILTIN, HIGH); // turn on the LED
    delay(500);                      // wait for half a second or 500 milliseconds
    digitalWrite(LED_BUILTIN, LOW);  // turn off the LED
    delay(500);                      // wait for half a second or 500 milliseconds
    Serial.println("LOOP");
}