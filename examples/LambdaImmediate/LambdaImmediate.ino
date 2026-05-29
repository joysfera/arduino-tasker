/*
 * Tasker Modern Example - setImmediate with Lambdas
 *
 * This is one of the most powerful new features in v3.
 * setImmediate runs the task on the *next* call to tasker.loop().
 *
 * Perfect for:
 *   - Deferring work without blocking
 *   - Breaking up long operations
 *   - Clean "run this soon" semantics (like JS setImmediate)
 */

#include "Tasker.h"

TaskerT<10> tasker;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println(F("=== setImmediate + Lambda Demo ==="));

  // 1. Simple immediate lambda
  tasker.setImmediate([]() {
    Serial.println("1. Immediate lambda ran");
  });

  // 2. Immediate that schedules more work (very useful pattern)
  tasker.setImmediate([]() {
    Serial.println("2. First immediate - scheduling follow-up...");

    tasker.setImmediate([]() {
      Serial.println("   2a. Follow-up immediate executed");
    });
  });

  // 3. Immediate used to "escape" from an ISR or time-critical section
  // (you can call setImmediate from anywhere)
  Serial.println("3. About to schedule from setup()...");

  tasker.setImmediate([]() {
    Serial.println("   3. Ran after setup() finished");
  });

  Serial.println("setup() complete. Entering loop...\n");
}

void loop() {
  tasker.loop();

  // You can also call setImmediate from normal code
  static uint32_t last = 0;
  if (millis() - last > 3000) {
    last = millis();
    tasker.setImmediate([]() {
      Serial.print(millis());
      Serial.println(" - Immediate from loop()");
    });
  }
}
