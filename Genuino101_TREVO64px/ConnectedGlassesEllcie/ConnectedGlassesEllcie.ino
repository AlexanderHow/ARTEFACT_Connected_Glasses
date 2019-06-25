#include <CurieBLE.h>
#include <math.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("5017f584-c96e-46c7-b337-b90b60a0147e");
BLECharacteristic trame("5017f584-c96e-46c7-b337-b90b60a0147e", BLERead | BLENotify, 20);

int depth_array[64] = {0};
bool hasDepthMapToSend = false;

void parse_depth_array(){
  //TODO : check trame[0] = 0x11 => hi +lo blabla remplir depth_array; then readyToParse = false + hasToSend = true
  int c;
  int hi;
  int byteRead = 0;
  int indexDepth = 0;
  bool goodIdTrame = false;
  while(Serial1.available() && (c = (int)Serial1.read()) != 0x0A){
    if(byteRead == 0){
      goodIdTrame = (c == 0x11) ? true : false;
    }else if(goodIdTrame == true && byteRead < 129){
      if(byteRead%2 == 1){
        hi = c;
      }else{
        int distance = ((hi << 7) | (c & 0x7F)) & 0x0FFF;
        if(distance > 0x1388){ //0x3FFF means the object is too far for the sensor
          distance = 0x1388;
        }
        depth_array[indexDepth] = map(distance, 0, 5000, 0, 255);
        indexDepth ++;
      }
    }else{
      //rest of the trame, last 8 bytes are the CRC32 checksum  
    }
    byteRead++;
  }
  hasDepthMapToSend = true;
}

void parse_depth_array_mock_up(){
  //TODO : remplissage random
}

void send_depth_array(){
  //TODO : 8 send BLE
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
