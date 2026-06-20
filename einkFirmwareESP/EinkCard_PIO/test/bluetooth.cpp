#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup()
{
    Serial.begin(115200);   
    SerialBT.begin("ESP32test");
    Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop()
{
    if (Serial.available()) {
        SerialBT.write(Serial.read());
    }
    if (SerialBT.available()) {
        Serial.write(SerialBT.read());
    }
    delay(20);
}