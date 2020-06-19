#include <Display.h>
#include <Logger.h>
#include <SerialLoggerTarget.h>

void setup() {
    Serial.begin(115200);
    Logger.init("test");
    SerialLoggerTarget *slt = new SerialLoggerTarget("Serial", 0);
    Logger.addLoggerTarget(slt);
    Logger.info("Main", "DisplayTest");
    Logger.info("Main", "===========");
    Display.init(ANIMATION_NONE, true, TO_LONG_BEHAVIOUR_NEWLINE);
}

int cnt = 0;
int number = 0;
void loop() {
    if(cnt > 1000){
        cnt = 0;
        Display.print("Message#: ");
        Display.println(number);
        number++;
    }
    cnt++;
    delay(1);
}
