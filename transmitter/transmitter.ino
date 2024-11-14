#include "BLEDevice.h"
#include "mbedtls/aes.h"

// BLE server address and UUIDs
static BLEAddress serverAddress("34:5f:45:aa:5d:fe");
static BLEUUID serviceUUID("38d36a74-e1b0-4d6c-b2ce-01f798c1b537");
static BLEUUID charUUID("294219fc-fe05-408d-9a8c-9571af83e78d");

static boolean connected = false;
static boolean commandSent = false; 
static BLERemoteCharacteristic *pRemoteCharacteristic = nullptr;
BLEClient *pClient = nullptr; 

const char *aesKey = "ef-robotics123456";

int counter = 0;
const int n_of_attempts = 10;

// Variables for 10-second RSSI monitoring
bool monitorRssi = false;
unsigned long rssiStartTime = 0;
int rssiSum = 0;
int rssiCount = 0;

// Function to set BLE transmit power level
void setBLEPowerLevel(int powerLevel) {
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, (esp_power_level_t)powerLevel);        // Set for advertising
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, (esp_power_level_t)powerLevel);  // Set for connection
  Serial.println("Client: Power level set");
}

class ClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {
    connected = true;
    Serial.println("Client: Connected to server.");
  }

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    commandSent = false; 
    Serial.println("Client: Disconnected from server.");
  }
};

// AES encryption function
void encryptAES(uint8_t *output, const uint8_t *input, const uint8_t *key) {
  mbedtls_aes_context aes;
  mbedtls_aes_init(&aes);
  mbedtls_aes_setkey_enc(&aes, key, 128); 
  mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);
  mbedtls_aes_free(&aes);
}

bool connectToServer() {
  Serial.print("Client: Attempting to connect to ");
  Serial.println(serverAddress.toString().c_str());

  pClient = BLEDevice::createClient();
  pClient->setClientCallbacks(new ClientCallback());

  if (!pClient->connect(serverAddress)) { 
    Serial.println("Client: Failed to connect to BLE server.");
    return false;
  }

  Serial.println(" - Connected to server.");
  pClient->setMTU(517);

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("Client: Failed to find our service UUID.");
    pClient->disconnect();
    return false;
  }

  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {
    Serial.println("Client: Failed to find our characteristic UUID.");
    pClient->disconnect();
    return false;
  }

  return true;
}

void cmd_open_door() {

  uint8_t plaintext[16] = {0}; 
  memcpy(plaintext, "OPEN_DOOR", 9);
  int rssi = pClient->getRssi();
  memcpy(plaintext + 9, &rssi, sizeof(rssi));
  uint8_t ciphertext[16];
  encryptAES(ciphertext, plaintext, (uint8_t *)aesKey);

  pRemoteCharacteristic->writeValue(ciphertext, sizeof(ciphertext));
  Serial.printf("Client: Command Sent! RSSI: %d\n", rssi);
}

void setup() {
  Serial.begin(115200);
  Serial.println("Client: Starting BLE Client application...");
  BLEDevice::init("");

  // Set transmit power level (e.g., ESP_PWR_LVL_N2 or ESP_PWR_LVL_N8 to reduce range)
  setBLEPowerLevel(ESP_PWR_LVL_P9);  // Use ESP_PWR_LVL_N12 for lowest power, ESP_PWR_LVL_P9 for highest

  if (connectToServer()) {
    Serial.println("Client: Successfully connected to BLE server.");
    counter = 0;
  } else {
    Serial.println("Client: Failed to connect to BLE server.");
  }
}


void loop() {
  if (connected && !commandSent && counter <= n_of_attempts) {
    cmd_open_door(); // Send the command with RSSI once upon connection
    //counter++;
    if(counter > n_of_attempts){
      commandSent = true;
      Serial.printf("Client: Command tranmission done %d times.\n", n_of_attempts);
      counter = 0; //reset
    }
     
  }

  if (!connected) {
    if (connectToServer()) {
      Serial.println("Reconnected to BLE server.");
      counter = 0;
    }
    delay(5000); // Retry connection every 5 seconds if disconnected
  }

  delay(1000); 
}
