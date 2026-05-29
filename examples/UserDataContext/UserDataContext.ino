/*
 * Tasker Modern Example - Using setUserData() with a Class
 *
 * This is the recommended pattern when you want to use Tasker
 * inside a C++ class (very common on ESP32, Teensy, RP2040, etc.).
 *
 * Technique:
 *   1. tasker.setUserData(this);
 *   2. Use a captureless lambda (or static function)
 *   3. Inside the callback: cast getUserData() back to your class
 */

#include "Tasker.h"

class LedController {
public:
  LedController(uint8_t pin, uint16_t intervalMs)
    : _pin(pin), _interval(intervalMs) {}

  void begin() {
    pinMode(_pin, OUTPUT);

    // Give the tasker a pointer to this instance
    tasker.setUserData(this);

    // Use a captureless lambda that calls back into the class
    tasker.setInterval([]() {
      // This is safe because we control the lifetime
      LedController* self = (LedController*) tasker.getUserData();
      self->toggleLed();
    }, _interval);
  }

  void toggleLed() {
    _state = !_state;
    digitalWrite(_pin, _state);

    // Optional: demonstrate we can also schedule from inside the class
    if (_blinkCount < 5) {
      _blinkCount++;
      // We can even schedule another immediate task
      tasker.setImmediate([]() {
        LedController* self = (LedController*) tasker.getUserData();
        self->printStatus();
      });
    }
  }

  void printStatus() {
    Serial.print(F("  [LedController on pin "));
    Serial.print(_pin);
    Serial.print(F("] state="));
    Serial.print(_state);
    Serial.print(F("  blinks="));
    Serial.println(_blinkCount);
  }

  // Public so the lambda above can see it
  TaskerT<6> tasker;

private:
  uint8_t  _pin;
  uint16_t _interval;
  bool     _state = false;
  uint8_t  _blinkCount = 0;
};

// =======================================================

LedController left(LED_BUILTIN, 400);
LedController right(12, 750);

void setup() {
  Serial.begin(115200);
  delay(100);

  Serial.println(F("=== Tasker + Class + setUserData() Demo ==="));

  left.begin();
  right.begin();

  Serial.println(F("Two independent LedControllers running with their own Taskers.\n"));
}

void loop() {
  left.tasker.loop();
  right.tasker.loop();
}
