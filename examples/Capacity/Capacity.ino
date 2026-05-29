/*
 * Tasker Modern Example - Capacity Management + setImmediate
 *
 * Demonstrates:
 *   - TaskerT<4> (very small capacity for demo)
 *   - isFull(), available(), count()
 *   - setImmediate() with captureless lambda
 *   - What happens when you exceed capacity (set* returns false)
 */

#include "Tasker.h"

TaskerT<4> tasker;   // Only 4 tasks allowed in this demo

void printStatus() {
  Serial.print(F("Tasks: "));
  Serial.print(tasker.count());
  Serial.print(F(" / "));
  Serial.print(tasker.capacity);
  Serial.print(F("   available: "));
  Serial.println(tasker.available());
}

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println(F("\n=== Tasker Capacity + setImmediate Demo ==="));
  printStatus();

  // Schedule 3 normal tasks
  tasker.setInterval([]() { Serial.println("  interval task"); }, 1000);
  tasker.setTimeout([]() { Serial.println("  timeout task"); }, 2500);
  tasker.setRepeated([]() { Serial.println("  repeated x3"); }, 400, 3);

  printStatus();  // Should show 3/4

  // Now use setImmediate (runs on next loop())
  tasker.setImmediate([]() {
    Serial.println(">>> IMMEDIATE lambda executed!");
  });

  printStatus();

  // Try to add one more than capacity
  bool ok = tasker.setInterval([]() {}, 100);
  if (!ok) {
    Serial.println(F("*** Could not add 5th task (capacity full) ***"));
  }

  Serial.println(F("\nLooping..."));
}

void loop() {
  tasker.loop();

  static uint32_t last = 0;
  if (millis() - last > 2000) {
    last = millis();
    printStatus();
  }
}
