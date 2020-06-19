#include <DisplayLogger.h>


void setup() {
    Serial.begin(115200);
    Serial.println("DisplayLogger");
    Serial.println("=============");
    DisplayLogger.init(ANIMATION_NONE, true, TO_LONG_BEHAVIOUR_NEWLINE);
}

int cnt = 0;
int abc = 0;
void loop() {
    if(cnt > 1000){
        cnt = 0;
        // DisplayLogger.println("Log 567890123456789012345678901234567890123456789012345678901234567");
        DisplayLogger.print("Log 567890123456: ");
        DisplayLogger.println(abc++);
    }
    cnt++;
    delay(1);
}
