/*
 * Example of usage of the Tasker for Arduino
 * Written by Petr Stehlik  petr@pstehlik.cz
 * Released into the public domain
 */

// define max number of tasks to save precious Arduino RAM
#define TASKER_MAX_TASKS 4
#include "Tasker.h"

Tasker tasker;

/*
 * Example of chaining tasks by calling the setTimeout() from a task
 */
void blink1()
{
    byte pin = 13;
    bool led = !digitalRead(pin);
    digitalWrite(pin, led);
    tasker.setTimeout(blink1, led ? 300 : 700);
}

/*
 * Example of one function called by two tasks - the 'pin'
 * input value is defined when creating the tasks
 */
void blink2(int pin)
{
    digitalWrite(pin, !digitalRead(pin));
}

void setup()
{
    pinMode(13, OUTPUT);  // default Arduino LED
    pinMode(12, OUTPUT);  // attach additional LED
    pinMode(11, OUTPUT);  // attach additional LED

    // called once after 5000 milliseconds, no optional parameter used
    tasker.setTimeout(blink1, 5000);
    // called every 750 milliseconds forever, with optional parameter pin = 12
    tasker.setInterval(blink2, 750, 12);
    // called every 1000 milliseconds 10 times, with optional parameter pin = 11
    tasker.setRepeated(blink2, 1000, 10, 11);
}

void loop()
{
    tasker.loop();
}

