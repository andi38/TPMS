/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
BLEUtils utils;

char convertCharToHex(char ch)
{
  char returnType;
  switch(ch)
  {
    case '0':
    returnType = 0;
    break;
    case  '1' :
    returnType = 1;
    break;
    case  '2':
    returnType = 2;
    break;
    case  '3':
    returnType = 3;
    break;
    case  '4' :
    returnType = 4;
    break;
    case  '5':
    returnType = 5;
    break;
    case  '6':
    returnType = 6;
    break;
    case  '7':
    returnType = 7;
    break;
    case  '8':
    returnType = 8;
    break;
    case  '9':
    returnType = 9;
    break;
    case  'a':
    returnType = 10;
    break;
    case  'b':
    returnType = 11;
    break;
    case  'c':
    returnType = 12;
    break;
    case  'd':
    returnType = 13;
    break;
    case  'e':
    returnType = 14;
    break;
    case  'f' :
    returnType = 15;
    break;
    default:
    returnType = 0;
    break;
  }
  return returnType;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    String addr = advertisedDevice.getAddress().toString().c_str();
    String mfdata = advertisedDevice.getManufacturerData().c_str();
    uint8_t* payload = advertisedDevice.getPayload();
    uint8_t payloadlen = advertisedDevice.getPayloadLength();
    char *payloadhex = utils.buildHexData(nullptr, payload, payloadlen);
    if (addr.startsWith("ac:15:85")) {  // my sensors MAC start with ac:15:85
      Serial.print("Payload: "); Serial.print(payloadhex);

      Serial.print("  ADDR: "); Serial.print(addr.substring(12));
      char *pHex = utils.buildHexData(nullptr, (uint8_t*)advertisedDevice.getManufacturerData().data(), advertisedDevice.getManufacturerData().length());
      Serial.print("  MFG DATA: "); Serial.print(pHex);
      String sHex = (String) pHex;
      byte nib[16];
      sHex.getBytes(nib,15);
      for (int i=0; i<14; i++) {
        nib[i] = convertCharToHex(nib[i]);
      }
      float Press = (float)((nib[7]*256+nib[8]*16+nib[9])-145)/10.0;
      String sPress = (String)Press;
      Serial.print("  p: "); Serial.print(sPress.substring(0,sPress.length()-1));
      int Temp = nib[4]*16+nib[5];
      Serial.print("  T: "); Serial.print(Temp);
      float Batt = (float)(nib[2]*16+nib[3])/10.0;
      String sBatt = (String)Batt;
      Serial.print("  b: "); Serial.print(sBatt.substring(0,sBatt.length()-1));
      Serial.print("  BIN: ");
      for (int i=0; i<2; i++) {prtnib(nib[i]);}
      Serial.print(".");
      for (int i=2; i<4; i++) {prtnib(nib[i]);}
      Serial.print(".");
      for (int i=4; i<6; i++) {prtnib(nib[i]);}
      Serial.print(".");
      for (int i=6; i<10; i++) {prtnib(nib[i]);}
      Serial.print(".");
      for (int i=10; i<14; i++) {prtnib(nib[i]);}

      bool nl = false;
      if (nib[0]==8) {Serial.print("   ALARM"); nl=true;}
      if (nib[0]==4) {Serial.print("   ROTAT"); nl=true;}
      if (nib[0]==2) {Serial.print("   STILL"); nl=true;}
      if (nib[0]==1) {Serial.print("   BGROT"); nl=true;}
      if (nib[1]==8) {Serial.println("   DECR2"); nl=false;}
      if (nib[1]==4) {Serial.println("   RISIN"); nl=false;}
      if (nib[1]==2) {Serial.println("   DECR1"); nl=false;}
      if ((nib[0]*16+nib[1])==0xff) {Serial.println("   LBATT");}
      if (nl) {Serial.println();}
    }
  }
};

void prtnib(int n) {
  if (n>=8) {Serial.print("1"); n-=8;} else {Serial.print("0");}
  if (n>=4) {Serial.print("1"); n-=4;} else {Serial.print("0");}
  if (n>=2) {Serial.print("1"); n-=2;} else {Serial.print("0");}
  if (n>=1) {Serial.print("1");} else {Serial.print("0");}
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
}
