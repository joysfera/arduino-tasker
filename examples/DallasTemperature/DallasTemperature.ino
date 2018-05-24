// 
// Example of non-blocking reading of DS18B20 temperature sensor
//

#include "Tasker.h"
#include "OneWire.h"
#include "DallasTemperature.h"

Tasker tasker;
OneWire oneWire(4);
DallasTemperature sensor(&oneWire);

void readSensor() {
    // read the actual temperature after it's been converted
    float temperature = sensor.getTempC(0);
    // do what you need with the temperature here
}

void startConversion() {
    // start temperature conversion (does not block)
    sensor.requestTemperatures();
    // schedule reading the actual temperature in 750 milliseconds
    tasker.setTimeout(readSensor, 750);
}

void setup() {
    sensor.begin();
    // do not block during temperature conversion
    sensor.setWaitForConversion(false);
    // read temperature every 5 seconds
    tasker.setInterval(startConversion, 5000);
}

void loop() {
    tasker.loop();
}
