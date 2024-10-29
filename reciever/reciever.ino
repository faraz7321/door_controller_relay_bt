#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

const int door_open_delay = 3000;      // Delay for relay in ms
const int cooldownDuration = 30000;    // Duration to ignore further commands
bool isCoolingDown = false;
unsigned long lastTriggerTime = 0;
unsigned long relayOnStartTime = 0;    // Timer for relay ON duration

const int relay1_pin = 16;             // GPIO for relay1 control
const int relay2_pin = 17;             // GPIO for relay2 control

String allowedDeviceKey = "DEV_7321_KEY";
bool authenticated = false;
bool relayActive = false;  // Tracks if relay is currently ON

void init_pins();
void trigger_relay(int relay_pin);

void setup() {
  Serial.begin(115200); // Serial Baud rate

  init_pins();

  if (!SerialBT.begin("ESP32_Relay")) {
    Serial.println("BT Connection: Error Connecting!");
  } else {
    Serial.println("BT Connection: Bluetooth Started!");
  }
}

void loop() {
  
  // Check if the cooldown period has expired
  if (isCoolingDown && (millis() - lastTriggerTime >= cooldownDuration)) {
    isCoolingDown = false;
    Serial.println("Cooldown period ended, ready for new commands.");

    // Clear any remaining data in the Bluetooth buffer
    while (SerialBT.available()) {
      SerialBT.read();  // Discard any remaining characters in the buffer
    }
  }

  // Check if the relay has been ON for the specified duration
  if (relayActive && (millis() - relayOnStartTime >= door_open_delay)) {
    digitalWrite(relay1_pin, LOW);  // Turn relay off
    Serial.println("Relay OFF");
    relayActive = false;

    // Start the cooldown period
    isCoolingDown = true;
    Serial.printf("Cooling down for %d milliseconds\n", cooldownDuration);
    lastTriggerTime = millis();  // Record the time when cooldown starts
  }

  // Only proceed if there is data and we're not in cooldown
  if (SerialBT.available() && !isCoolingDown) {
    String command = SerialBT.readStringUntil('\n');  // Read the command
    command.trim();

    // Check if the device is authenticated
    if (!authenticated) {
      if (command == allowedDeviceKey) {
        authenticated = true;
        Serial.println("Device authenticated, you may now send commands.");
      } else {
        Serial.println("Authentication failed, ignoring commands.");
        // Clear buffer in case of invalid attempt
        while (SerialBT.available()) {
          SerialBT.read();
        }
        return;  // Exit loop to prevent further command processing
      }
    }

    // Process command only if authenticated
    if (authenticated) {
      if (command == "DEV_7321_KEY_TRIGGER") {
        trigger_relay(relay1_pin);
      } else {
        Serial.println("Unknown command");
      }
    }

    // Clear any remaining data in the Bluetooth buffer
    while (SerialBT.available()) {
      SerialBT.read();  // Discard any remaining characters in the buffer
    }
  }
}

void init_pins() {
  // Initialize relay pins
  pinMode(relay1_pin, OUTPUT);
  pinMode(relay2_pin, OUTPUT);

  digitalWrite(relay1_pin, LOW);  // Relay off initially
  digitalWrite(relay2_pin, LOW);
}

void trigger_relay(int relay_pin) {
  digitalWrite(relay_pin, HIGH);  // Turn relay on
  Serial.printf("Relay ON for %d milliseconds\n", door_open_delay);

  relayOnStartTime = millis();  // Start timer for relay ON duration
  relayActive = true;           // Mark relay as active
}
