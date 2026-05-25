#include "Arduino.h"
#include "SoftwareSerial.h"

#define BT_RX 7
#define BT_TX 6
SoftwareSerial bluetooth(BT_TX, BT_RX);


void setup()
{
    Serial.begin(9600);
    bluetooth.begin(38400);
    Serial.println("starting");
}

void loop()

{

    if (bluetooth.available())

    Serial.write(bluetooth.read());

    if (Serial.available())

    bluetooth.write(Serial.read());

}