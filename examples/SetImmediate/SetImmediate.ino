/*
 * SetImmediate example for Tasker v3
 *
 * Demonstrates the new setImmediate() function added in v3.
 * setImmediate() schedules a task to run as soon as possible
 * (higher priority than normal timed tasks).
 */

#include "Tasker.h"

// Use TaskerT for explicit small size (good practice in v3)
TaskerT<6> tasker;

int counter = 0;

void urgentTask() {
    Serial.print("URGENT #");
    Serial.println(++counter);
}

void normalTask() {
    Serial.println("normal task");
}

void setup() {
    Serial.begin(115200);
    Serial.println("Tasker v3 - setImmediate demo");

    // This will run as soon as possible (before normalTask)
    tasker.setImmediate(urgentTask);

    // Normal interval task
    tasker.setInterval(normalTask, 1000);

    // You can also pass parameters
    tasker.setImmediate([](int value) {
        Serial.print("Immediate with value: ");
        Serial.println(value);
    }, 42);
}

void loop() {
    tasker.loop();
}
