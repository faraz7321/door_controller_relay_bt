#include "HardwareSerial.h"
#include "esp32-hal.h"
#include "DoorServer.h"
#include "mbedtls/aes.h"

// Constructor
DoorServer::DoorServer(DoorController *controller,
                       const char *serviceUUID,
                       const char *characteristicUUID,
                       const char *aesKey,
                       int rssiThreshold,
                       unsigned long rssiMonitorDuration)
  : doorController(controller),
    serviceUUID(serviceUUID),
    characteristicUUID(characteristicUUID),
    aesKey(aesKey),
    rssiThreshold(rssiThreshold),
    rssiMonitorDuration(rssiMonitorDuration),
    rssi(0),
    rssiSum(0),
    rssiCount(0),
    monitorRssi(false),
    rssiStartTime(0) {}

void DoorServer::setup() {
  BLEDevice::init("esp32_door_server");
  setBLEPowerLevel(ESP_PWR_LVL_P9);

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallbacks(doorController));

  pService = pServer->createService(serviceUUID);
  pCharacteristic = pService->createCharacteristic(
    characteristicUUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new Callbacks(this));
  pService->start();

  startAdvertising();
}
void DoorServer::setRssi(int newRssi) {
  rssi = newRssi;
  if (rssi < rssiThreshold && !monitorRssi) {
    startRssiMonitoring();
  }
}
// Implementation for processRssiMonitoring()
void DoorServer::processRssiMonitoring() {
  if (monitorRssi) {
    rssiSum += rssi;
    rssiCount++;

    if (millis() - rssiStartTime >= rssiMonitorDuration) {
      float averageRssi = static_cast<float>(rssiSum) / rssiCount;
      if (averageRssi < rssiThreshold && doorController->getDoorPosition() == DoorController::DOOR_OPENED && !doorController->getIsCoolingdown()) {
        Serial.println("Server: Closing door due to low RSSI");
        doorController->closeDoor();
        Serial.println("Server: Door closed due to low RSSI.");
      }
      resetRssiMonitoring();
    }
  }
}

// Implementation for setBLEPowerLevel()
void DoorServer::setBLEPowerLevel(int powerLevel) {
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, (esp_power_level_t)powerLevel);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, (esp_power_level_t)powerLevel);
}

// Implementation for startAdvertising()
void DoorServer::startAdvertising() {
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
}

// AES decryption function
void DoorServer::decryptAES(uint8_t *output, const uint8_t *input) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_dec(&aes, (const unsigned char *)aesKey, 128);
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_DECRYPT, input, output);
  mbedtls_aes_free(&aes);
}
void DoorServer::startRssiMonitoring() {
  Serial.println("Server: Threshold low, started monitoring RSSI");
  monitorRssi = true;
  rssiStartTime = millis();
  rssiSum = rssi;
  rssiCount = 1;
}

// Reset RSSI monitoring variables
void DoorServer::resetRssiMonitoring() {
  Serial.println("Server: Reset RSSI monitoring");
  monitorRssi = false;
  rssiSum = 0;
  rssiCount = 0;
}

// Implementation for Callbacks::onWrite
void Callbacks::onWrite(BLECharacteristic *pCharacteristic) {

  uint8_t decryptedData[16];
  char command[10] = { 0 };
  int receivedRssi;

  String encryptedData = pCharacteristic->getValue();
  if (encryptedData.length() != 16) {
    Serial.println("Server: Invalid command length");
    return;
  }

  
  if (!doorServer->doorController->getIsCoolingdown()) {
  doorServer->decryptAES(decryptedData, (uint8_t *)encryptedData.c_str());
  memcpy(command, decryptedData, 9);  
  memcpy(&receivedRssi, decryptedData + 9, sizeof(receivedRssi));

  Serial.printf("Server: Command received with RSSI: %d\n", receivedRssi);
  doorServer->setRssi(receivedRssi);
  }
  else {
    Serial.println("Server: Relay Cooldown in process.");
    return;
  }

  if (strcmp(command, "OPEN_DOOR") == 0 && receivedRssi >= doorServer->rssiThreshold) {
    if (doorServer->doorController->getDoorPosition() == DoorController::DOOR_CLOSED && !doorServer->doorController->getIsCoolingdown()) {
        Serial.println("Server: Command accepted, opening door.");
        doorServer->doorController->openDoor();
        Serial.println("Server: Door opened by the client.");
      }
  } else {
    Serial.println("Server: Command rejected or RSSI below threshold.");
  }
}

// Implementation for ServerCallbacks::onConnect
void ServerCallbacks::onConnect(BLEServer *pServer) {
  Serial.println("Server: Client connected");
}

// Implementation for ServerCallbacks::onDisconnect
void ServerCallbacks::onDisconnect(BLEServer *pServer) {
  Serial.println("Server: Client disconnected");

  if (doorController->getDoorPosition() == DoorController::DOOR_OPENED && !doorController->getIsCoolingdown()) {
    Serial.println("Server: Closing door upon no connection");
    doorController->closeDoor();
    Serial.println("Server: Door closed!");
  }
  delay(60000);
  BLEDevice::startAdvertising();
  Serial.println("Server: Searching for connections...");
}
