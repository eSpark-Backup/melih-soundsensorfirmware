#include <NimBLEDevice.h>

extern "C" {void app_main(void);}

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint32_t value = 0;
char* msg;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

/**  None of these are required as they will be handled by the library with defaults. **
 **                       Remove as you see fit for your needs                        */  
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer, BLEConnInfo& connInfo) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer, BLEConnInfo& connInfo, int reason) {
      deviceConnected = false;
    }
/***************** New - Security handled here ********************
****** Note: these are the same return values as defaults ********/
  uint32_t onPassKeyRequest(){
    printf("Server PassKeyRequest\n");
    return 123456; 
  }

  bool onConfirmPIN(uint32_t pass_key){
    printf("The passkey YES/NO number: %d\n", pass_key);
    return true; 
  }

  void onAuthenticationComplete(BLEConnInfo& connInfo){
    printf("Starting BLE work!\n");
  }
/*******************************************************************/
};

void connectedTask (void * parameter){
    for(;;) {
        // notify changed value
        if (deviceConnected) 
        {
            if(value < 10)
            {
              msg = "True";
            }
            else if(value < 20)
            {
              msg = "False";
            }
            else
            {
              value = 0;
            }
            pCharacteristic->setValue((uint8_t *)msg, strlen(msg));
            pCharacteristic->notify();
            value++;
            vTaskDelay(100/portTICK_PERIOD_MS);  // bluetooth stack will go into congestion, if too many packets are sent
        }
        // disconnecting
        if (!deviceConnected && oldDeviceConnected) {
            vTaskDelay(500/portTICK_PERIOD_MS); // give the bluetooth stack the chance to get things ready
            pServer->startAdvertising(); // restart advertising
            printf("start advertising\n");
            oldDeviceConnected = deviceConnected;
        }
        // connecting
        if (deviceConnected && !oldDeviceConnected) {
            // do stuff here on connecting
            oldDeviceConnected = deviceConnected;
        }
        
        vTaskDelay(10/portTICK_PERIOD_MS); // Delay between loops to reset watchdog timer
    }
    
    vTaskDelete(NULL);
}
    
void app_main(void) {
  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      NIMBLE_PROPERTY::INDICATE
                    );

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  
  xTaskCreate(connectedTask, "connectedTask", 5000, NULL, 1, NULL);
  
  BLEDevice::startAdvertising();
  printf("Waiting a client connection to notify...\n");
}