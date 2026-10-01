// BLE demo for ESP32-C6: control the RGB LED and read temperature / BOOT button
// from a browser via Web Bluetooth (see ../ble_demo.html).
#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID "6e1f0001-6c0f-4b8e-9c5a-2d6b0c6e1f00"
#define LED_UUID     "6e1f0002-6c0f-4b8e-9c5a-2d6b0c6e1f00"  // write/read: 3 bytes r,g,b
#define TEMP_UUID    "6e1f0003-6c0f-4b8e-9c5a-2d6b0c6e1f00"  // read/notify: int16 centi-degC
#define BUTTON_UUID  "6e1f0004-6c0f-4b8e-9c5a-2d6b0c6e1f00"  // read/notify: uint8 1=pressed

#ifndef RGB_BUILTIN
#define RGB_BUILTIN 8
#endif

BLEServer *server;
BLECharacteristic *ledChar, *tempChar, *buttonChar;
volatile bool connected = false, wasConnected = false;

class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *) override {
    connected = true;
    Serial.println("Client connected");
  }
  void onDisconnect(BLEServer *) override {
    connected = false;
    Serial.println("Client disconnected");
  }
};

class LedCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *c) override {
    String v = c->getValue();
    if (v.length() < 3) return;
    uint8_t r = v[0], g = v[1], b = v[2];
    rgbLedWrite(RGB_BUILTIN, r, g, b);
    Serial.printf("LED %u %u %u\n", r, g, b);
  }
};

void setup() {
  Serial.begin(115200);
  Serial.setTxTimeoutMs(0);
  pinMode(BOOT_PIN, INPUT_PULLUP);
  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);

  BLEDevice::init("C6-BLE-Demo");
  server = BLEDevice::createServer();
  server->setCallbacks(new ServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);
  ledChar = service->createCharacteristic(LED_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  ledChar->setCallbacks(new LedCallbacks());
  uint8_t off[3] = {0, 0, 0};
  ledChar->setValue(off, 3);

  tempChar = service->createCharacteristic(TEMP_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  tempChar->addDescriptor(new BLE2902());
  buttonChar = service->createCharacteristic(BUTTON_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  buttonChar->addDescriptor(new BLE2902());
  service->start();

  // 128-bit UUID goes in the advertisement, the name in the scan response.
  BLEAdvertising *adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(SERVICE_UUID);
  adv->setScanResponse(true);
  BLEDevice::startAdvertising();
  Serial.println("Advertising as C6-BLE-Demo");
}

void loop() {
  static uint32_t lastTemp = 0;
  static int lastButton = -1;

  int pressed = digitalRead(BOOT_PIN) == LOW;
  if (pressed != lastButton) {
    lastButton = pressed;
    uint8_t v = pressed;
    buttonChar->setValue(&v, 1);
    if (connected) buttonChar->notify();
  }

  if (millis() - lastTemp >= 1000) {
    lastTemp = millis();
    int16_t centi = (int16_t)(temperatureRead() * 100);
    tempChar->setValue((uint8_t *)&centi, 2);
    if (connected) tempChar->notify();
  }

  // Advertising stops on connect; restart it after the client leaves.
  if (!connected && wasConnected) {
    delay(500);
    server->startAdvertising();
    Serial.println("Advertising again");
  }
  wasConnected = connected;
  delay(20);
}
