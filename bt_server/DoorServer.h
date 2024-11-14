#ifndef DOORSERVER_H
#define DOORSERVER_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "DoorController.h"

class DoorServer {
private:
    BLEServer *pServer;
    BLEService *pService;
    BLECharacteristic *pCharacteristic;
    DoorController *doorController;
    const char *serviceUUID;
    const char *characteristicUUID;
    const char *aesKey;

    int rssi;
    int rssiSum;
    int rssiCount;
    bool monitorRssi;
    unsigned long rssiStartTime;
    const int rssiThreshold;
    const unsigned long rssiMonitorDuration;

    friend class Callbacks;

public:
    DoorServer(DoorController *controller, const char *serviceUUID, const char *characteristicUUID, const char *aesKey, int rssiThreshold, unsigned long rssiMonitorDuration);

    void setup();
    void startAdvertising();
    void setRssi(int newRssi);
    void startRssiMonitoring();
    void processRssiMonitoring();
    void resetRssiMonitoring();
    void decryptAES(uint8_t *output, const uint8_t *input);
    static void setBLEPowerLevel(int powerLevel);
};

class Callbacks : public BLECharacteristicCallbacks {
private:
    DoorServer *doorServer;

public:
    Callbacks(DoorServer *server) : doorServer(server) {}
    void onWrite(BLECharacteristic *pCharacteristic) override;
};

class ServerCallbacks : public BLEServerCallbacks {
private:
    DoorController *doorController;

public:
    ServerCallbacks(DoorController *controller) : doorController(controller) {}
    void onConnect(BLEServer *pServer) override;
    void onDisconnect(BLEServer *pServer) override;
};

#endif
