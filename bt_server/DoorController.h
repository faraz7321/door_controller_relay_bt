#ifndef DOORCONTROLLER_H
#define DOORCONTROLLER_H

#include <Arduino.h>

class DoorController {
public:
enum DoorPosition {
    DOOR_CLOSED,
    DOOR_OPENED,
    DOOR_OPENING,
    DOOR_CLOSING,
  };

private:
  const int relayPin;
  const int relayCooldownPeriod = 30;
  bool isCoolingdown = false;
  DoorPosition doorPosition;

public:
  DoorController(int relayPin);
  void triggerRelay(int seconds);
  DoorPosition getDoorPosition() const;      
  void setDoorPosition(DoorPosition position); 
  bool closeDoor();
  bool openDoor();
  void relayCooldown(int seconds);
  bool getIsCoolingdown();
  void setIsCoolingDown(bool state);
};

#endif
