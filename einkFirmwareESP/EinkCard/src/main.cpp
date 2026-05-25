#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <SPI.h>
#include "epd4in2_V2.h"

#define BTN_1 22
#define BTN_2 21

#define COMMS_CHUNK_SIZE 511
#define COMMS_OK 0xFF

// BLE Device Name
#define SERVER_NAME "Eink Card"

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
bool btn1Pressed = false, btn2Pressed = false;
int lastIntr = 0;
bool deviceConnected = false; 

Epd epd;

void IRAM_ATTR btn1Interrupt()
{ 
  if ( millis() - lastIntr < 100) { return; }
  btn1Pressed = true;
  lastIntr = millis();
}

void IRAM_ATTR btn2Interrupt()
{
  if ( millis() - lastIntr < 100) { return; }
  btn2Pressed = true;
  lastIntr = millis();
}

u_int imageSize = 0;
u_int receivedByteCount = 0;
//max image buffer, with 4 color grayscale
uint8_t imageBuffer[EPD_WIDTH*EPD_HEIGHT/4] = {0};

void getStatusBytes(float voltage, bool btn1, bool btn2, uint8_t *statusByte);
float readVcc();
void setupBLE();

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t* param) {      
      uint8_t *arr = pCharacteristic->getData();
      int arrLen = pCharacteristic->getLength();      
      //received size of image
      if (imageSize == 0)
      {        
        memcpy(&imageSize, arr, arrLen);        
        Serial.println("receiving start");
      }
      else if (receivedByteCount < imageSize)
      {
        memcpy((imageBuffer+receivedByteCount), arr, arrLen);
        receivedByteCount += arrLen;                
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("connected to a device");
    deviceConnected = true;
    bleServer->getAdvertising()->stop();
    
  };
  void onDisconnect(BLEServer* pServer) {
    Serial.println("disconnected");
    deviceConnected = false;
    bleServer->getAdvertising()->start();
  }

  void onMtuChanged(BLEServer* pServer, esp_ble_gatts_cb_param_t* param)
  {
    Serial.print("mtu updated");
  }
};

void setupBLE() {
  BLEDevice::init(SERVER_NAME);
  BLEDevice::setMTU(517);
  bleServer = BLEDevice::createServer();  
  bleServer->setCallbacks(new MyServerCallbacks());

  BLEService *service = bleServer->createService(SERVICE_UUID);
  bleImage = service->createCharacteristic(
    IMAGE_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  bleImage->setCallbacks(new MyCallbacks());  
  bleImage->setValue("test");  

  bleButton1 = service->createCharacteristic(
    BTN1_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );
  bleButton2 = service->createCharacteristic(
    BTN2_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY
  );

  service->start();
  BLEAdvertising *advertising = BLEDevice::getAdvertising();
  advertising->addServiceUUID(SERVICE_UUID);
  bleServer->getAdvertising()->start(); 
}

void setup()
{
  
  Serial.begin(115200);    
  Serial.println("startign");

  pinMode(BTN_1, INPUT_PULLUP);
  attachInterrupt(BTN_1, btn1Interrupt, RISING);
  pinMode(BTN_2, INPUT_PULLUP);
  attachInterrupt(BTN_2, btn2Interrupt, RISING);

  if (epd.Init() != 0) {
    Serial.print("e-Paper init failed");
    return;
  }
  epd.Clear();  

  epd.SendCommand(0x24);
  for (int i = 0;i < 480*100;i++)
  {
    epd.SendData(0xF0);
  }
  epd.TurnOnDisplay();



  setupBLE();
  Serial.println("all ok!");
}


void loop()
{   
  //skip if device is not connected
  if (!deviceConnected)
  {
    Serial.println("device not connected");
    delay(1000);
  }

  if (btn1Pressed)
  {
    btn1Pressed = false;
    Serial.println("button 1 pressed");        
    bleButton1->notify();
  }
  else if (btn2Pressed)
  {
    btn2Pressed = false;
    Serial.println("button 2 pressed");    
    bleButton2->notify();
  }  

  //received all the bytes for the image
  if (imageSize != 0 && receivedByteCount >= imageSize)
  {
    Serial.println("sending data!");
    
    epd.SendCommand(0x24);    
    for (int i = 0;i < imageSize;i++)
    {      
      epd.SendData(imageBuffer[i]);    
    }
    epd.TurnOnDisplay();

    receivedByteCount = 0;
    imageSize = 0;
  }
  delay(50);
}