#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#include <SPI.h>
#include "epd4in26.h"

#define BTN_1 22
#define BTN_2 21

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
  if ( millis() - lastIntr < 500) { return; }
  btn1Pressed = true;
  lastIntr = millis();
}

void IRAM_ATTR btn2Interrupt()
{
  if ( millis() - lastIntr < 500) { return; }
  btn2Pressed = true;
  lastIntr = millis();
}
enum ImageUpdateType {
  NONE = 0,
  BLACK_WHITE_FAST = 1,
  BLACK_WHITE = 2,
  FOUR_GRAY_FAST = 3,
  FOUR_GRAY = 4,
};

enum ImageUpdateType imageUpdating = NONE;
u_int imageBytesToSend = 0;
void setupBLE();

class ImageCharCallback: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic, esp_ble_gatts_cb_param_t* param) {      
      uint8_t *arr = pCharacteristic->getData();
      int arrLen = pCharacteristic->getLength();      

      //receiving new image
      if (imageUpdating == NONE)
      {     
        imageUpdating = (ImageUpdateType)arr[0];
        switch (imageUpdating)
        {
        case BLACK_WHITE_FAST:
          imageBytesToSend = (EPD_HEIGHT * EPD_WIDTH)/8;
          epd.SendCommand(0x24);
        case BLACK_WHITE:
          imageBytesToSend = (EPD_HEIGHT * EPD_WIDTH)/8;         
          epd.SendCommand(0x24);
          break;

        case FOUR_GRAY_FAST:
          break;
        case FOUR_GRAY:          
          break;
        
        default:
          break;
        }
      }
      else 
      {
        for (int i = 0;i < arrLen;i++)
        { epd.SendData(arr[i]); }
        imageBytesToSend -= arrLen;        
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
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  bleImage->setCallbacks(new ImageCharCallback());

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
  if (imageBytesToSend == 0 && imageUpdating != NONE)
  {
    Serial.println("refreshing");
    switch (imageUpdating)
    {
    case BLACK_WHITE_FAST:
    Serial.println("BW FAST");
      imageUpdating = NONE;
      epd.TurnOnDisplay_Part();
      break;
      
    case BLACK_WHITE:
      Serial.println("BW");
      imageUpdating = NONE;
      epd.TurnOnDisplay(); 
      break;
    
    default:
      break;
    }
  }
  delay(50);
}