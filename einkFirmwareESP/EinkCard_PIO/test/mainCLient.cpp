//  BLE Client Example Sketch
//
//  Programming Electronics Academy
//
#include <Arduino.h>
#include <BLEDevice.h>            // sets up BLE device constructs

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00000000-0000-0000-0000-000000000000");

static BLEUUID imageUUID("cba1d466-344c-4be3-ab3f-189f80dd7518");
static BLERemoteCharacteristic* imageChar;
static BLEUUID btn1UUID("f78ebbff-c8b7-4107-93de-889a6a06d408");
static BLERemoteCharacteristic* btn1Char;
static BLEUUID btn2UUID("ca73b3ba-39f6-4ab3-91ae-186dc9577d99");
static BLERemoteCharacteristic* btn2Char;

static boolean connected = false;
static BLEAdvertisedDevice* myDevice = nullptr;


static void imageNotified(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  Serial.println("Notified!");
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
    Serial.println("onConnect");
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    myDevice = nullptr;
    Serial.println("onDisconnect");
  }
};

bool connectToServer() {
  Serial.print("Forming a connection to ");
  Serial.println(myDevice->getAddress().toString().c_str());
  BLEClient*  pClient  = BLEDevice::createClient();
  Serial.println(" - Created client");
  pClient->setClientCallbacks(new MyClientCallback());

  // Connect to the remove BLE Server.
  pClient->connect(myDevice);
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {

    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    pClient->disconnect();

    return false;
  }
  Serial.println(" - Found our service");

  // imageChar = pRemoteService->getCharacteristic(imageUUID);
  // btn1Char = pRemoteService->getCharacteristic(btn1UUID);
  // btn1Char = pRemoteService->getCharacteristic(btn2UUID);
  // if (imageChar == nullptr || btn1Char == nullptr || btn2Char == nullptr) {
  //   Serial.print("Failed to find our characteristics");    
  //   pClient->disconnect();
  //   return false;
  // }

  // if(imageChar->canNotify())
  //   imageChar->registerForNotify(imageNotified);
  connected = true;
  return true;

}

// Scan for BLE servers and find the first one that advertises the service we are looking for.
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

  // Called for each advertising BLE server.
  void onResult(BLEAdvertisedDevice advertisedDevice) {

    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // We have found a device, let us now see if it contains the service we are looking for.
    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
  }

};

void startScan()
{
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device.  Specify that we want active scanning and start the
  // scan to run for 5 seconds.

  Serial.println("starting scan");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
}

void setup() {

  Serial.begin(115200);
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  if (!connected)
    startScan();

} // End of setup.

// This is the Arduino main loop function that runs repeatedly.

void loop() {
  if (!connected && myDevice == nullptr) 
    startScan();
  else if (!connected && myDevice != nullptr) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {

      Serial.println("We have failed to connect to the server; there is nothin more we will do.");

    }
  }

  Serial.println("loop");

  // If we are connected to a peer BLE Server, update the characteristic each time we are reached
  // with the current time since boot.

  // if (connected) {

  //   String newValue = "Time since boot: " + String(millis()/1000);
  //   Serial.println("Setting new characteristic value to \"" + newValue + "\"");

  //   // Set the characteristic's value to be the array of bytes that is actually a string.
  //   pRemoteCharacteristic->writeValue(newValue.c_str(), newValue.length());

  // }else if(doScan){

  //   BLEDevice::getScan()->start(0);  // this is just an example to re-start the scan after disconnect

  // }

  delay(1000); // Delay a second between loops.

}