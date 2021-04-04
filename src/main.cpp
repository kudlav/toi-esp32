#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEAddress.h"
#include "BLEScan.h"

// the number of the LED pin
#define ledRed 14
#define ledGreen 12
#define ledWhite 13

// setting PWM properties
#define freq 5000
#define ledChannelRed 0
#define ledChannelGreen 1
#define ledChannelWhite 2
#define resolution 10

// UUID služby zařízení
static BLEUUID serviceUUID("180f");
// UUID jednotlivých vlastností daného zařízení
static BLEUUID    charUUID_NOTIFY("1110");
static BLEUUID    charUUID_RED("1000");
static BLEUUID    charUUID_GREEN("0100");
static BLEUUID    charUUID_BLUE("0010");


static BLEAddress* DEV_ADDR = new BLEAddress("b8:27:eb:81:47:16"); // MAC Raspberry

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic_notify;
static BLERemoteCharacteristic* pRemoteCharacteristic_red;
static BLERemoteCharacteristic* pRemoteCharacteristic_green;
static BLERemoteCharacteristic* pRemoteCharacteristic_blue;
static BLEAdvertisedDevice* myDevice;

//Funkce pro reakci na notifikační zprávu
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify
) {
  Serial.print("Notify callback");
/*
  const char* notify = pRemoteCharacteristic_notify->readValue().c_str();
  Serial.print("notify: ");
  Serial.println(notify);
*/
  std::string red = pRemoteCharacteristic_red->readValue();
  Serial.print("red: ");
  Serial.println(red.c_str());
  ledcWrite(ledChannelRed, atoi(red.c_str()));

  std::string green = pRemoteCharacteristic_green->readValue();
  Serial.print("green: ");
  Serial.println(green.c_str());
  ledcWrite(ledChannelGreen, atoi(green.c_str()));

  std::string blue = pRemoteCharacteristic_blue->readValue();
  Serial.print("blue: ");
  Serial.println(blue.c_str());
  ledcWrite(ledChannelWhite, atoi(blue.c_str()));

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
    pRemoteCharacteristic_notify = pRemoteService->getCharacteristic(charUUID_NOTIFY);
    if (pRemoteCharacteristic_notify == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_NOTIFY.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- notify");

    //Testování přípojení se ke vlastnosti GATT na BLE zařízení
    pRemoteCharacteristic_blue = pRemoteService->getCharacteristic(charUUID_BLUE);
    if (pRemoteCharacteristic_blue == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_BLUE.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- blue");

    //Testování přípojení se ke vlastnosti GATT na BLE zařízení
    pRemoteCharacteristic_green = pRemoteService->getCharacteristic(charUUID_GREEN);
    if (pRemoteCharacteristic_green == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_GREEN.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- green");

    //Testování přípojení se ke vlastnosti GATT na BLE zařízení
    pRemoteCharacteristic_red = pRemoteService->getCharacteristic(charUUID_RED);
    if (pRemoteCharacteristic_red == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID_RED.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic -- red");

    // Testování zda daná vlastnost má možnost čtení, případně přečtení první hodnoty
    if(pRemoteCharacteristic_green->canRead()) {
      std::string value = pRemoteCharacteristic_green->readValue();
      Serial.print("The characteristic value green: ");
      Serial.println(value.c_str());
      ledcWrite(ledChannelGreen, atoi(value.c_str()));
    }

    // Testování zda daná vlastnost má možnost čtení, případně přečtení první hodnoty
    if(pRemoteCharacteristic_red->canRead()) {
      std::string value = pRemoteCharacteristic_red->readValue();
      Serial.print("The characteristic value red: ");
      Serial.println(value.c_str());
      ledcWrite(ledChannelRed, atoi(value.c_str()));
    }

    // Testování zda daná vlastnost má možnost čtení, případně přečtení první hodnoty
    if(pRemoteCharacteristic_blue->canRead()) {
      std::string value = pRemoteCharacteristic_blue->readValue();
      Serial.print("The characteristic value blue: ");
      Serial.println(value.c_str());
      ledcWrite(ledChannelWhite, atoi(value.c_str()));
    }

    //Testování zda UUID má vlastnost notifikačních zpráv
    //Nastavení třídy, která bude reagovat na notifikační zprávy.
    if(pRemoteCharacteristic_notify->canNotify())
      pRemoteCharacteristic_notify->registerForNotify(notifyCallback);

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

  // configure LED PWM functionalitites
  ledcSetup(ledChannelRed, freq, resolution);
  ledcSetup(ledChannelGreen, freq, resolution);
  ledcSetup(ledChannelWhite, freq, resolution);

  // attach the channel to the GPIO to be controlled
  ledcAttachPin(ledRed, ledChannelRed);
  ledcAttachPin(ledGreen, ledChannelGreen);
  ledcAttachPin(ledWhite, ledChannelWhite);

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
}
