#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEAddress.h"
#include "BLEScan.h"


// UUID služby zařízení
static BLEUUID serviceUUID("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
// UUID jednotlivých vlastností daného zařízení
static BLEUUID    charUUID("6e400002-b5a3-f393-e0a9-e50e24dcca9e");
static BLEUUID    charUUID_RX("6e400003-b5a3-f393-e0a9-e50e24dcca9e");


static BLEAddress* DEV_ADDR = new BLEAddress("b8:27:eb:e3:86:db"); // MAC Raspberry

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLERemoteCharacteristic* pRemoteCharacteristicRX;
static BLEAdvertisedDevice* myDevice;

//Funkce pro reakci na notifikační zprávu
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    Serial.println((char*)pData);
}


class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};


// Funkce pro připojení se BLE serveru a všem jeho vlastnostem.
bool connectToServer() {
    Serial.print("Forming a connection to ");
    Serial.println(myDevice->getAddress().toString().c_str());
    
    //Nastavení klienta pro pro komunikaci přes BLE
    BLEClient*  pClient  = BLEDevice::createClient();
    Serial.println(" - Created client");

    //Nastavení třidy reagující na připojení případné odpojení se od BLE zařízení
    pClient->setClientCallbacks(new MyClientCallback());

    //Připojení se k bluetooth serveru
    pClient->connect(myDevice); 
    Serial.println(" - Connected to server");

    //Testování přípojení se ke službě GATT na BLE zařízení
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");


    //Testování přípojení se ke vlastnosti GATT na BLE zařízení
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- transmiter");

    //Testování přípojení se ke vlastnosti GATT na BLE zařízení
    pRemoteCharacteristicRX = pRemoteService->getCharacteristic(charUUID_RX);
    if (pRemoteCharacteristicRX == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_RX.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- receiver");

    // Testování zda daná vlastnost má možnost čtení, případně přečtení první hodnoty
    if(pRemoteCharacteristicRX->canRead()) {
      std::string value = pRemoteCharacteristicRX->readValue();
      Serial.print("The characteristic value was: ");
      Serial.println(value.c_str());
    }

    //Testování zda UUID má vlastnost notifikačních zpráv
    //Nastavení třídy, která bude reagovat na notifikační zprávy.
    if(pRemoteCharacteristicRX->canNotify())
      pRemoteCharacteristicRX->registerForNotify(notifyCallback);


    connected = true;
    return true;
}


class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
   * Zavoláno při nalezení jakéhokoli BLE zařízení
   * Zpracvování dat BLE zařízení
   */
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    // Porovnání adress zařízení pro zjištění daného zařízní
    if(DEV_ADDR->equals(advertisedDevice.getAddress())){
      //Možnost dohledávání zařízení podle poskytujících službách
      //if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
          Serial.print("CONNECTING"); 
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = true;

    } 
  } 
};


void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // vyckej
  }

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("UZ01");

  // Zjištění adresy vlasntního zařízení
  BLEAddress myAddr = BLEDevice::getAddress();
  myAddr.toString();


  //Hledání zařízení
  BLEScan* pBLEScan = BLEDevice::getScan();
  //Definice třídy, pro zpracování jednotlivých dat o skenovaném zařízení
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  //Probíhání skenování bluetooth zařízení po dobu 5 sekund
  pBLEScan->start(5, false);
}

void loop() {
  
  //Proveď připojení k BLE zařízení pokud zařízení bylo nalezeno
  if (doConnect == true) {
    if (connectToServer()) {
      Serial.println("We are now connected to the BLE Server.");
    } else {
      Serial.println("We have failed to connect to the server; there is nothin more we will do.");
    }
    doConnect = false;
  }

  String data;  
  if (connected) {
    //Získání řetězce zprávy
    data = Serial.readString();
    String newValue = "Time since boot: " + String(millis()/1000);
    printf("%s ---- data: %s\n", newValue.c_str(), data.c_str());
    
    if(data == "")
      ;
    else{
      //Nastavení a poslání zprávy
      pRemoteCharacteristic->writeValue(data.c_str(), data.length());
      printf("Sending message: %s\n", data.c_str());
    }
    
    /*
      //Možnost zeptání se na hodnotu dané vlastnosti (Vlastnost musí podporovat hodnotu READ)
      std::string inData = pRemoteCharacteristicRX->readValue();
      Serial.println(inData.c_str());
    */
  }  
}