/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

//https://randomnerdtutorials.com/esp32-ble-server-client/

#include <Arduino.h> 
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID2 "beb5483e-36e1-4688-b7f5-ea07deadbeef"

int counter = 0; 
BLECharacteristic *pCharacteristic, *pCharacteristic2;
BLEServer *pServer;
BLEService *pService;

bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("### Device connected!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("### Device disconnected!");
    }
};

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      Serial.println("### onWrite");
      String uuidinfo = pCharacteristic->getUUID().toString().c_str();
      Serial.println("UUID: "+uuidinfo);
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);
        Serial.println();
        Serial.println("*********");
      }

    }
};

void blinkInternalLed(int count){
  for (int i = 0; i < count; i++)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting BLE work!");

  BLEDevice::init("Pauls Boxing Bot");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY
                                         //BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic2 = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID2,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_NOTIFY |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setValue("Hello World says Paul");
  pCharacteristic2->setValue("Here you can write");
  
  //pCharacteristic2->setCallbacks(new CharacteristicCallBack());
  pCharacteristic2->setCallbacks(new MyCharacteristicCallbacks());
  
  pService->start();
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  blinkInternalLed(5);
}

void loop() {
  BLEDevice::startAdvertising();
  //Serial.println("Characteristic defined! Now you can read it in your phone!");

  // put your main code here, to run repeatedly:
  String currentTimeHex = String(millis(), HEX);
  String jsonValue = currentTimeHex+","+counter;
  counter++;
  Serial.println(jsonValue);
  pCharacteristic->setValue(jsonValue.c_str());
  String connectedStr = deviceConnected? "true":"false";
  pCharacteristic->notify();
  Serial.println("Connected: "+connectedStr);

  String pCharacteristic2Value = pCharacteristic2->getValue().c_str();
  Serial.println("pCharacteristic2Value: "+pCharacteristic2Value);
  delay(2000);

}