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
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Tasker: Demo of setInterval()");

    // call my function 'task1' every 3 seconds
    tasker.setInterval(task1, 3000);
}

void loop()
{
    tasker.loop();
    // your code can be here
}

