#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// BLE Device Name
#define DEVICE_NAME "ESP32_BLE_Server"

// Custom UUIDs
#define SERVICE_UUID        "00000000-0000-0000-0000-000000000000"
#define CHARACTERISTIC_UUID "00000000-0000-0000-0000-0000000b0000"

BLEServer *bleServer;
BLECharacteristic *characteristic;

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("some value arrived");
    }

    bool get_bit(uint32_t num, uint32_t position)
    {
      bool bit = num & (1 << position);
      return bit;
    }

    void print_binary(uint32_t num)
    {
      Serial.print("binary: ");
      for(int i = 31; i >= 0; i--)
      {
        Serial.print(get_bit(num, i));
      }
      Serial.println(" -- ");
    }
};

void setup()
{
    Serial.begin(115200);

    BLEDevice::init(DEVICE_NAME);
    // Create the BLE server
    bleServer = BLEDevice::createServer();

    // Create the service and characteristic
    BLEService *service = bleServer->createService(SERVICE_UUID);
    characteristic = service->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    characteristic->setCallbacks(new MyCallbacks());

    // Set an initial value
    characteristic->setValue("Initializing...");
  
    // Start the service
    service->start();

    // Make the device discoverable
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    bleServer->getAdvertising()->start();    
    
    Serial.println("BLE server started");

}

void loop()
{
    float temperature = random(20, 30) + 0.1 * random(0, 10);
    String value = "Aemp: " + String(temperature) + "°C";

    // Update the characteristic value
    characteristic->setValue(value.c_str());
    characteristic->notify(); // Notify connected clients
    Serial.println(value);
    delay(1000);

}