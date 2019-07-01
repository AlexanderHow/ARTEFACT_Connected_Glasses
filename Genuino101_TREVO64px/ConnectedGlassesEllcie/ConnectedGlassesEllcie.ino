#include <CurieBLE.h>
#include <math.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("5017f584-c96e-46c7-b337-b90b60a0147e");
BLECharacteristic trame("5017f584-c96e-46c7-b337-b90b60a0147e", BLERead | BLENotify, 20);

int depth_array[64] = {0};
bool hasDepthMapToSend = false;

void parse_depth_array(){
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
      //rest of the trame, last 8 bytes are the CRC32 checksum or wrong id
    }
    byteRead++;
  }
  hasDepthMapToSend = true;
}

void parse_depth_array_mock_up(){
  for(int i = 0; i < 64; ++i){
    depth_array[i] = random(0,255);  
  }
  hasDepthMapToSend = true;
  Serial.println("GENERATED");
}

void send_depth_array(){
  unsigned char toSend[20] = {0};
  toSend[0] = 0x11;
  toSend[2] = 8;
  for(int j = 0; j < 8; ++j){
    toSend[1] = (char)j;
    for(int k = 0; k < 8; ++k){
      toSend[k+3] = (char)depth_array[8*j+k];  
    }
    trame.setValue(toSend, 20);
    delay(100);
  }
  hasDepthMapToSend = false;
  Serial.println("SENT");
}

void setup() {
  Serial.begin(9600);
  delay(100);
  
  ///TERA EVO 64px UART
  //Serial1.begin(115200);
  //Serial1.write(0x0011024C); //Distance mode
  delay(100);
  
  ///BLE
  blePeripheral.setLocalName("ARTEFACT");
  blePeripheral.setAdvertisedServiceUuid(glassesService.uuid());
  blePeripheral.addAttribute(glassesService);
  blePeripheral.addAttribute(trame);
  blePeripheral.begin();
  delay(100);  
}

void loop() {
  BLECentral central = blePeripheral.central();
  if (central) {
    while (central.connected()) {
      if(hasDepthMapToSend == true){
         send_depth_array();
      }else{
         parse_depth_array_mock_up(); //TO CHANGE WHEN WE HAVE THE ACTUAL SENSOR
      }
      delay(500);
    }
  }

}
