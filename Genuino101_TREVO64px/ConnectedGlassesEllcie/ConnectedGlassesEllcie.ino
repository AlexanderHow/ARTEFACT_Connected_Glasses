#include <CurieBLE.h>
#include <math.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc");
BLECharacteristic trame("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc", BLERead | BLENotify, 20);

int depth_array[64] = {0};
uint8_t frame[137] = {0}; //1 byte header + 64*2 bytes of data + 8 bytes of CRC32
int indexFrame = 0;
int indexCRC = 0;
bool hasDepthMapToSend = false;
bool paddingEnded = false;

void parseEvo(){
  uint8_t header;
  uint16_t distance;
  header = frame[0];
  if(header == 17){
    for(int i = 1; i < 65; ++i){
      //Serial.print("frame ");Serial.print(frame[2*i-1]);Serial.print(" ");Serial.print(frame[2*i]); <= show that we retrieve incoherent data from sensor
      distance = (frame[2*i-1] & 0x7F) << 7;
      distance = distance | (frame[2*i] & 0x7F);
      // ou distance = (frame[2*i-1]<<8) + frame[2*i];
      distance = (distance > 5000) ? 5000 : distance;
      distance = map(distance, 0, 5000, 0, 255);
      depth_array[i-1] = distance;
    }
    //crc32 only on the 4 lower bits of each
    /*for(int j = 129 ; j < 137; ++j){
      uint8_t crc = frame[j] (& 0x0F);
    }*/
    hasDepthMapToSend = true;
   }  
}

void send_depth_array(){
  unsigned char toSend[20] = {0};
  toSend[0] = 17; //0x11
  toSend[2] = 8;
  for(int j = 0; j < 8; ++j){
    toSend[1] = (char)j;
    for(int k = 0; k < 8; ++k){
      toSend[k+3] = (char)depth_array[8*j+k];  
    }
    trame.setValue(toSend, 20);
    delay(80);
  }
  hasDepthMapToSend = false;
}

void setup() {
  /*BLE*/
  blePeripheral.setLocalName("ARTEFACT");
  blePeripheral.setAdvertisedServiceUuid(glassesService.uuid());
  blePeripheral.addAttribute(glassesService);
  blePeripheral.addAttribute(trame);
  blePeripheral.begin(); 
  delay(100);
  
  Serial.begin(115200);
  delay(100);
  
  /*TERA EVO 64px UART*/
  Serial1.begin(115200);
  delay(100);
  Serial1.write("\x00\x52\x02\x01\xDF"); //activate VCP
  delay(100);
  Serial1.flush();
  Serial1.write("\x00\x11\x02\x4C"); //distance mode
  delay(100);
  Serial1.flush();
  Serial1.write("\x00\x21\x02\xB5"); //fast mode
  delay(100);
  Serial1.flush();
}

void loop() {
  BLECentral central = blePeripheral.central();
  uint8_t recv;
  if (central) {
    while (central.connected()) {
      if(indexFrame == 129 && indexCRC == 8){
        parseEvo();
        if(hasDepthMapToSend == true){
           send_depth_array();
        }  
        indexFrame = 0; 
        indexCRC = 0;
        paddingEnded = false; 
      }else{
        while(Serial1.available()>0){
          recv = Serial1.read();
          if(indexFrame == 0){ //header
            if(recv == 17){
              frame[indexFrame] = recv;
              indexFrame++;
            }
          }
          else if(indexFrame > 0 && indexFrame < 129){ //distance data
            frame[indexFrame] = recv;
            indexFrame++;
          }
          else{ //padding and crc32
            if(paddingEnded == true){
              if(indexCRC < 8){
                frame[indexFrame + indexCRC] = recv;
                indexCRC++;
              }else{
                break;
              }
            }else{
              if(recv != 128){ //0x80 = padding
                frame[indexFrame + indexCRC] = recv;
                indexCRC++;
                paddingEnded = true;
              }
            }
          }
        }
      }
    }
    indexFrame = 0; 
    indexCRC = 0;
    paddingEnded = false;
    Serial1.flush();
  }else{
    indexFrame = 0; 
    indexCRC = 0;
    paddingEnded = false;
    Serial1.flush();  
  }
}
//DEBUG
/*void printDepth(){
  for(int i = 0; i < 8; ++i){
    for(int j = 0; j < 8; ++j){
      Serial.print(depth_array[i*8+j]);Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println("_______________"); 
}*/
