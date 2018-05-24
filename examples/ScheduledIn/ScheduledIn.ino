/*
 * Example of usage of the Tasker for Arduino
 * Written by Petr Stehlik  petr@pstehlik.cz
 * Released into the public domain
 */

#include "Tasker.h"
Tasker tasker;

void task1()
{
    static byte counter = 0;
    Serial.print(millis());
    Serial.print(F(": task1 called: "));
    counter++;
    Serial.println(counter);
}

void task2()
{
    unsigned long ms = tasker.scheduledIn(task1);
    if (ms > 0) {
        Serial.print(F("task1 will be called in ")); Serial.print(ms); Serial.println(F(" milliseconds"));
    }
    else {
        Serial.println("task1 will not be called anymore");

        // stop task2 as it is no longer needed
        tasker.cancel(task2);
    }
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Tasker: Demo of setInterval(), setRepeated(), scheduledIn() and cancel()");

    // schedule task1 to be called every 4 seconds but five times only
    tasker.setRepeated(task1, 4000, 5);

    // schedule task2 to be called every 500 milliseconds
    tasker.setInterval(task2, 500);
}

void loop()
{
    tasker.loop();
    // your code can be here
}

