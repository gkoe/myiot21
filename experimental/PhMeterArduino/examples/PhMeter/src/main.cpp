#include <DisplayLogger.h>

const float V9_18 = 1550.0; // mV
const float V6_86 = 0.0;
const float V4_01 = 2070.0;
const float V0_00 = 2654; // 2,654V bei ph-Wert 0
const float V24_50 = 0.0; // 0V bei ph-Wert 24,5

const float k = (9.18 - 4.01) / ((V9_18 - V4_01) / 1000.0);
const float d = 9.18 - (k*V9_18/1000.0);


void setup()
{
    Serial.begin(115200);
    Serial.println();
    Serial.println("PH-Meter");
    Serial.println("========");
    DisplayLogger.init(ANIMATION_NONE, true, TO_LONG_BEHAVIOUR_NEWLINE);
    Serial.print("k:");
    Serial.print(k);
    Serial.print(", d:");
    Serial.println(d);
    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);
}

// const float k = -9.22 / 1000; // mv/ph-Wert
// const float d = 24.5;
char line[25];
int cnt = 1;

void loop()
{
    int value = analogRead(36);  // ADC0
    float ph = (k/1000.0 * value + d);
    sprintf(line, "%d mV: %i, ph: %.1f", cnt, value, ph);
    DisplayLogger.print(cnt);
    DisplayLogger.print(" mV: ");
    DisplayLogger.print(value);
    DisplayLogger.print(", ph: ");
    DisplayLogger.println(ph);
    // DisplayLogger.println(line);
    cnt++;
    delay(5000);
}
