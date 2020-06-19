#include <DisplayLogger.h>


void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println("DisplayLogger");
    Serial.println("=============");
    DisplayLogger.init(ANIMATION_NONE, true, TO_LONG_BEHAVIOUR_NEWLINE);
}

int cnt = 0;
int number = 0;
void loop() {
    if(cnt > 1000){
        cnt = 0;
        // DisplayLogger.println("Log 567890123456789012345678901234567890123456789012345678901234567");
        DisplayLogger.print("Log: ");
        DisplayLogger.println(number++);
    }
    cnt++;
    delay(1);
}
