#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "mbedtls/aes.h"

#define SERVICE_UUID "38d36a74-e1b0-4d6c-b2ce-01f798c1b537"
#define CHARACTERISTIC_UUID "294219fc-fe05-408d-9a8c-9571af83e78d"

// GPIO pin connected to the relay
const int relayPin = 17;

const char *aesKey = "ef-robotics12345";

// AES decryption function
void decryptAES(uint8_t *output, const uint8_t *input, const uint8_t *key) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, key, 128);  // Set AES decryption key (128-bit)
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input, output);
  mbedtls_aes_free(&aes);
}
void trigger_relay(int relayPin, int seconds) {

  Serial.println("Door opened\n");
  digitalWrite(relayPin, HIGH);  
  delay(seconds *1000);                  
  digitalWrite(relayPin, LOW);
  Serial.println("Door closed\n");
  delay(60000);
}

// Class to handle BLE characteristic write events
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue();  // Get the incoming data
    if (value.length() != 16) {                  // Verify the expected length of AES block size (16 bytes)
      Serial.println("Invalid command length");
      return;
    }

    uint8_t decryptedText[16];
    decryptAES(decryptedText, (uint8_t *)value.c_str(), (uint8_t *)aesKey);

    // Check if the decrypted message matches the expected "OPEN_DOOR" command
    if (memcmp(decryptedText, "OPEN_DOOR", 9) == 0) {
      Serial.println("Received command: OPEN_DOOR");
      trigger_relay(relayPin, 30);
    } else {
      Serial.println("Unknown command");
    }
  }
};

// Class to handle client connection and disconnection events
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Client connected");
  }

  void onDisconnect(BLEServer *pServer) {
    Serial.println("Client disconnected");
    BLEDevice::startAdvertising();  // Restart advertising after disconnection
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE Server");

  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);  // Ensure relay is off initially

  BLEDevice::init("esp32_door_server");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();

  // Start advertising the service
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Server started, waiting for commands...");
}

void loop() {
  // Main loop does nothing as everything is handled by BLE callbacks
  delay(1000);
}
