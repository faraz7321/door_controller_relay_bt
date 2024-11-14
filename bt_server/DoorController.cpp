#include "esp32-hal.h"
#include "HardwareSerial.h"
#include "DoorController.h"


DoorController::DoorController(int relayPin)
  : doorPosition(DOOR_CLOSED), relayPin(relayPin) {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);
}

DoorController::DoorPosition DoorController::getDoorPosition() const {
  return doorPosition;
}
bool DoorController::getIsCoolingdown(){
  return isCoolingdown;
}

void DoorController::setDoorPosition(DoorController::DoorPosition position) {
  doorPosition = position;
}

void DoorController::triggerRelay(int seconds) {
  digitalWrite(relayPin, HIGH);
  delay(seconds * 1000);
  digitalWrite(relayPin, LOW);
  relayCooldown(30);
}

bool DoorController::closeDoor() {

  if (getDoorPosition() == DOOR_OPENED) {

    Serial.println("Door Controller: Closing Door!");
    setDoorPosition(DoorController::DOOR_CLOSING);
    triggerRelay(1);
    Serial.println("Door Controller: Door Closed!");
    setDoorPosition(DoorController::DOOR_CLOSED);
    return true;
  }
  else {
    Serial.println("Door Controller: closeDoor() called. Door controller is busy");
    return false;
  }
}

bool DoorController::openDoor() {

  if (getDoorPosition() == DOOR_CLOSED) {

    Serial.println("Door Controller: Opening Door!");
    setDoorPosition(DoorController::DOOR_OPENING);
    triggerRelay(1);
    Serial.println("Door Controller: Door Opened!");
    setDoorPosition(DoorController::DOOR_OPENED);
    return true;
  }
  else {
    Serial.println("Door Controller: openDoor() called. Door controller is busy");
    return false;
  }
}

void DoorController::relayCooldown(int seconds){
  setIsCoolingDown(true);
  Serial.printf("Door Controller: Relay cooldown started for %d seconds\n", seconds);
  delay(seconds * 1000);
  Serial.println("Door Controller: Cooldown ended");
  setIsCoolingDown(false);
}

void DoorController::setIsCoolingDown(bool state){
  isCoolingdown = state;
}
