#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc");
BLECharacteristic trame("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc", BLERead | BLENotify, 20);

uint8_t * depth_array = NULL;
int indexDepth = 0;
bool initialDataStream = false;
int numLine = -1;
int numCol = -1;

//Sending one line of the depth map that changed as ( 0x 11 SN LL XX XX XX XX ... 00 or XX) size = 20 bytes
//with SN : Sequential number or index of the line, LL : Line length, XX : Depth data, 00 : Padding to make a 20 bytes long trame
void sendDepthArrayLine(int indexLine){
  if(indexLine < numLine && numCol < 17){
    unsigned char toSend[20] = {0};
    toSend[0] = 17; //0x11
    toSend[1] = (char)indexLine;    
    toSend[2] = numCol;
    for(int k = 0; k < numCol; ++k){
      toSend[k+3] = (char)depth_array[8*indexLine+k];  
    }
    trame.setValue(toSend, 20);
    delay(10);
  }
}

void setUpInitialParams(uint8_t recv){
  if( numLine <= 0){ 
    if( initialDataStream == true){ 
      numLine = (recv > 0) ? recv : -1;
    }
  }else if( numCol <= 0){ 
    if( initialDataStream == true){ 
      numCol = (recv > 0) ? recv : -1;
      if(depth_array != NULL){
        free(depth_array);  
      }
      depth_array = (uint8_t*)calloc(numLine*numCol, sizeof(uint8_t));
      initialDataStream = false;
    }
  }
}

void setup() {
  //BLE set up
  blePeripheral.setLocalName("ARTEFACT");
  blePeripheral.setAdvertisedServiceUuid(glassesService.uuid());
  blePeripheral.addAttribute(glassesService);
  blePeripheral.addAttribute(trame);
  blePeripheral.begin(); 
  delay(80);

  //DEBUG
  /*Serial.begin(115200);
  delay(80);*/

  //Serial(0,1) communication with module with the sensor, each message begin with 0x00 and continue by sending all the pixels of the depth map
  //Would need to be at 3MBps but impossible on this target
  Serial1.begin(115200);
  delay(80);
  while(!Serial1){;}
}

void loop() {
  uint8_t recv;
  BLECentral central = blePeripheral.central();
  if(Serial1.available() > 0){
    recv = (uint8_t)Serial1.read();

    if(recv == 255){ //Start of the stream about the sizes of the depth matrix
      initialDataStream = true;
    }
    
    if( numLine > 0 && numCol > 0){ //Stream about depth data, starting by a 0 and followed by 64 values
      if( recv == 0){ //Start of depth stream
        indexDepth = 0;
      }else{ //Depth data
        depth_array[indexDepth] = recv;
        indexDepth++;
        
        if(indexDepth != 0 && (indexDepth%numCol) == 0){ //Start of a new line in the depth matrix
          int indexLine = (indexDepth-1)/numLine;
          if( central && central.connected()){ //Send the line through BLE
            sendDepthArrayLine(indexLine);
          }
        }

        if(indexDepth >= (numLine*numCol)){ //Security to not overflow the matrix if some pieces of the data stream were missed
          indexDepth = (numLine*numCol) - 1;
        }
        
      }
    }else{ //Stream about the sizes of the depth matrix
      setUpInitialParams(recv);
    }
  }
}
