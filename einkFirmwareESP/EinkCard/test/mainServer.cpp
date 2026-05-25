#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <SPI.h>
#include "epd4in2_V2.h"

#define BTN_1 15
#define BTN_2 21

#define COMMS_CHUNK_SIZE 511
#define COMMS_OK 0xFF

// BLE Device Name
#define SERVER_NAME "Eink Server"

// Custom UUIDs
#define SERVICE_UUID "00000000-0000-0000-0000-000000000000"

#define IMAGE_UUID "cba1d466-344c-4be3-ab3f-189f80dd7518"
#define BTN1_UUID "f78ebbff-c8b7-4107-93de-889a6a06d408"
#define BTN2_UUID "ca73b3ba-39f6-4ab3-91ae-186dc9577d99"


//Address of the peripheral device. Address will be found during scanning...
BLEServer *bleServer;

BLECharacteristic *bleImage;
BLECharacteristic *bleButton1;
BLECharacteristic *bleButton2;
bool deviceConnected = false;
bool doconnect = false;

Epd epd;

bool previousBtn1State = 1;
bool previousBtn2State = 1;

ushort receivingImageSize = 0;
ushort receivedImageSize = 0;
//max image buffer, with 4 color grayscale
byte imageBuffer[EPD_WIDTH*EPD_HEIGHT/4] = {0};

void getStatusBytes(float voltage, bool btn1, bool btn2, uint8_t *statusByte);
float readVcc();
void setupBLE();

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("some value arrived");
    }
};

void setupBLE() {
  BLEDevice::init(SERVER_NAME);
  bleServer = BLEDevice::createServer();

  BLEService *service = bleServer->createService(SERVICE_UUID);
  bleImage = service->createCharacteristic(
    IMAGE_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  bleImage->setCallbacks(new MyCallbacks());  
  bleImage->setValue("test");

  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  bleServer->getAdvertising()->start(); 
}

void setup()
{
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  
  Serial.begin(115200);    
  Serial.println("startign");

  // if (epd.Init() != 0) {
  //   Serial.print("e-Paper init failed");
  //   return;
  // }
  // epd.Clear();    
  setupBLE();
  Serial.println("all ok!");
}


void loop()
{   
  //hang when bluetooth is not connected 
  // while (!deviceConnected) { delay(1); }

  previousBtn1State = digitalRead(BTN_1);
  previousBtn2State = digitalRead(BTN_2);
  delay(50);
}

// void recvAndSendBW()
// {
//     epd.SendCommand(0x24);
//     //BW images total 15KB    
    
//     int chunkCount = 15000/COMMS_CHUNK_SIZE;
//     int chunkRemainder = 15000%COMMS_CHUNK_SIZE;      
//     for (int i =0;i < chunkCount; i++)
//     {
//         //wait for full chunk size to come in
//         while (bluetooth.available() < COMMS_CHUNK_SIZE) { /*Wait for full chunk */}
//         for (int j = 0;j < COMMS_CHUNK_SIZE; j++)
//         {                         
//             epd.SendData(bluetooth.read());
//         }        
//         //send current chunk as OK
//         bluetooth.write(COMMS_OK);      
//     } 
//     while (bluetooth.available() < chunkRemainder) { /* Do nothing */ }
//     for (int j = 0;j < chunkRemainder; j++)
//         { epd.SendData(bluetooth.read()); }
        
//     epd.TurnOnDisplay();
// }


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