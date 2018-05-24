/*
 * Example of usage of the Tasker for Arduino
 * Written by Petr Stehlik  petr@pstehlik.cz
 * Released into the public domain
 */

#include "Tasker.h"
Tasker tasker;

void task1()
{
    Serial.print(millis());
    Serial.println(F(": task1 called"));

    unsigned r = random(1, 5);
    Serial.print(F("will call it again in ")); Serial.print(r); Serial.println(F(" seconds"));
    tasker.setTimeout(task1, r * 1000);
}

void setup()
{
    Serial.begin(115200);
    Serial.println(F("Tasker: Demo of chaining setTimeout() calls"));

    Serial.println(F("will call task1 in exactly 5 seconds"));
    tasker.setTimeout(task1, 5000);
}

void loop()
{
    tasker.loop();
    // your code can be here
}

