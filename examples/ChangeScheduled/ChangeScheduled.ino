/*
 * Example of usage of the Tasker for Arduino
 * Written by Petr Stehlik  petr@pstehlik.cz
 * Released into the public domain
 */

#include "Tasker.h"
Tasker tasker;

void task1()
{
    static unsigned long last = 0;
    Serial.print(millis());
    Serial.print(F(": task1 called after "));
    Serial.println(millis() - last);
    last = millis();
}

void task2()
{
    Serial.print(millis());
    Serial.print(F(": reset and change the interval of task1 to "));
    unsigned r = random(1000, 3000);
    Serial.println(r);
    tasker.setInterval(task1, r);
}

void task3()
{
    Serial.print(millis());
    Serial.println(": STOP");
    tasker.cancel(task1);
    tasker.cancel(task2);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Tasker: Demo of changing scheduled task setup at runtime");

    // set task1 to be called every 2 seconds
    tasker.setInterval(task1, 2000);
    
    // every 8 seconds call task2 that changes the settings of task1
    tasker.setInterval(task2, 8000);
    
    // after 60 seconds stop both tasks
    tasker.setTimeout(task3, 60000);
}

void loop()
{
    tasker.loop();
    // your code can be here
}

