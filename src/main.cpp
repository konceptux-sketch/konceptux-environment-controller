#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("Konceptux Relay16");
    Serial.println("Firmware started");
}

void loop() {
    Serial.println("ESP32 running...");
    delay(1000);
}