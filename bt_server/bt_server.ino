#include "DoorController.h"
#include "DoorServer.h"

const char *serviceUUID = "38d36a74-e1b0-4d6c-b2ce-01f798c1b537";
const char *characteristicUUID = "294219fc-fe05-408d-9a8c-9571af83e78d";
const char *aesKey = "";

const int relayPin = 17;               // GPIO pin connected to the relay
const int rssiThreshold = -100;         // RSSI threshold for range
const unsigned long rssiMonitorDuration = 10000; 

DoorController doorController(relayPin); 
DoorServer doorServer(&doorController, serviceUUID, characteristicUUID, aesKey, rssiThreshold, rssiMonitorDuration);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE Door Controller...");

    // Initialize the BLE server
    doorServer.setup();
}

void loop() {
    doorServer.processRssiMonitoring();
    delay(1000); 
}
