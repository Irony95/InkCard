#include <Arduino.h>
#include "BluetoothSerial.h"
#include <SPI.h>
#include "epd4in2_V2.h"

#define BTN_1 15
#define BTN_2 21

#define COMMS_CHUNK_SIZE 511
#define COMMS_OK 0xFF

BluetoothSerial bluetooth;
Epd epd;

int sendStatusPeriod = 2000;
int sendStatusDelta = 0;

bool previousBtn1State = 1;
bool previousBtn2State = 1;

void getStatusBytes(float voltage, bool btn1, bool btn2, uint8_t *statusByte);
float readVcc();
void recvAndSendBW();
void pauseBluetoothUnConnected()
{
    while (!bluetooth.connected())
    {
        Serial.println("BT not connected");
        delay(500);
    }
}

void setup()
{
    pinMode(BTN_1, INPUT_PULLUP);
    pinMode(BTN_2, INPUT_PULLUP);

    Serial.begin(115200);    
    bluetooth.begin("Eink Card");

    if (epd.Init() != 0) {
        Serial.print("e-Paper init failed");
        return;
    }
    epd.Clear();
    pauseBluetoothUnConnected();
    Serial.println("all ok!");
}


void loop()
{   
    //hang when bluetooth is not connected 
    pauseBluetoothUnConnected();

    if (digitalRead(BTN_1) != previousBtn1State || digitalRead(BTN_2) != previousBtn2State)
    {
        sendStatusDelta = sendStatusPeriod;
    }

    if (bluetooth.available())
    {
        uint8_t uploadFunc = bluetooth.read();
        switch (uploadFunc)
        {
            case 0x01:
                bluetooth.write(COMMS_OK);
                Serial.println("doing BW download");
                recvAndSendBW();                
                break;

            default:
                break;
        }
    }

    
    if (sendStatusDelta >= sendStatusPeriod)
    {        
        sendStatusDelta = 0;
        if (bluetooth.connected())
        {
            uint8_t statusBytes[3];
            getStatusBytes(readVcc(), !digitalRead(BTN_1), !digitalRead(BTN_2), statusBytes);

            bluetooth.write(statusBytes, 3);                        
            bluetooth.flush();
        }
    }

    sendStatusDelta += 50;
    previousBtn1State = digitalRead(BTN_1);
    previousBtn2State = digitalRead(BTN_2);
    delay(50);
}

void recvAndSendBW()
{
    epd.SendCommand(0x24);
    //BW images total 15KB    
    
    int chunkCount = 15000/COMMS_CHUNK_SIZE;
    int chunkRemainder = 15000%COMMS_CHUNK_SIZE;      
    for (int i =0;i < chunkCount; i++)
    {
        //wait for full chunk size to come in
        while (bluetooth.available() < COMMS_CHUNK_SIZE) { /*Wait for full chunk */}
        for (int j = 0;j < COMMS_CHUNK_SIZE; j++)
        {                         
            epd.SendData(bluetooth.read());
        }        
        //send current chunk as OK
        bluetooth.write(COMMS_OK);      
    } 
    while (bluetooth.available() < chunkRemainder) { /* Do nothing */ }
    for (int j = 0;j < chunkRemainder; j++)
        { epd.SendData(bluetooth.read()); }
        
    epd.TurnOnDisplay();
}


/*
3 bytes of status to be sent to the phone as communication
1st byte
bit 5 = btn 2
bit 4 = btn 1
bit 0-3 = voltage whole number, (i.e. 3V, 4V, 5V)

2nd byte
bit 0-7 = voltage mV
*/
void getStatusBytes(float voltage, bool btn1, bool btn2, uint8_t *statusByte)
{
    int voltageFloor = int(voltage);
    uint8_t firstByte = voltageFloor + (btn1 << 4) + (btn2 << 5);    
    uint8_t secondByte = int((voltage - voltageFloor)*100);

    statusByte[0] = 0xF1; 
    statusByte[1] = firstByte;
    statusByte[2] = secondByte;
}


float readVcc() 
{
  return 5.0;
}