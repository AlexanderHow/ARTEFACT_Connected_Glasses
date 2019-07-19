#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc");
BLECharacteristic trame("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc", BLERead | BLENotify, 20);

uint8_t * depth_array = NULL;
int indexDepth = 0;
bool startSetUpInitParams = false;
bool setUpDone = false;
bool startFrame = false;
int numLine = -1;
int numCol = -1;

void sendDepthArrayLine(int indexLine){
  //Sending one line of the depth map that changed as ( 0x 11 SN LL XX XX XX XX ... 00 or XX) size = 20 bytes
  //with SN : Sequential number or index of the line, LL : Line length, XX : Depth data, 00 : Padding to make a 20 bytes long trame
  if(numLine > 0 && numCol > 0 && indexLine < numLine && numCol < 17){
    unsigned char toSend[20] = {0};
    toSend[0] = 17; //0x11
    Serial.print(17);Serial.print(" ");
    toSend[1] = (char)indexLine;    
    Serial.print(indexLine);Serial.print(" ");
    toSend[2] = numCol;
    Serial.print(numCol);Serial.print(" ");
    for(int k = 0; k < numCol; ++k){
      toSend[k+3] = (char)depth_array[8*indexLine+k];
      Serial.print(depth_array[8*indexLine+k]);Serial.print(" ");  
    }
    trame.setValue(toSend, 20);
    Serial.println();
    delay(100);
  }
}

void setUpInitialParams(uint8_t input){
  if(startSetUpInitParams == false){
    if(input == 255){ //start of initial data trame
      startSetUpInitParams = true;  
    }
  }else{
    //setting the size of the matrix
    if(numLine == -1){ 
      numLine = (input > 0) ? input : -1;
    }else{
      numCol = (input > 0) ? input : -1;
    }
    //allocating the matrix
    if(numLine > 0 && numCol > 0){ 
      if(depth_array != NULL){
        free(depth_array);  
      }
      depth_array = (uint8_t*)calloc(numLine*numCol, sizeof(uint8_t));
      setUpDone = true;
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
  Serial.begin(115200);
  delay(80);

  //Serial(0,1) communication with module with the sensor, each message begin with 0x00 and continue by sending all the pixels of the depth map
  Serial1.begin(115200);
  delay(80);
  while(!Serial1){;}
  delay(100);
}

void loop() {
  BLECentral central = blePeripheral.central();
  uint8_t recv;
  while(setUpDone == false){ //INITIAL DATA : size of the matrix as (0x FF XX XX)
    if(Serial1.available() > 0){
      recv = (uint8_t)Serial1.read();
      setUpInitialParams(recv);  
    }
  }
  Serial.println("Alloc ok");
  //START BLE AND RECEVING DATA FROM SENSOR
  if (central) {
    while (central.connected()) {
      if(Serial1.available() > 0){
        recv = (uint8_t)Serial1.read(); 
        if(recv == 0){ //start trame
          indexDepth = 0;  
          startFrame = true;
        }else if(startFrame == true){ //depth data
          depth_array[indexDepth] = recv;
          indexDepth = (indexDepth + 1);
          if(indexDepth > 0 && (indexDepth%numCol) == 0){ //new line of the depth map
            sendDepthArrayLine((indexDepth-1)/numCol);
          }
          indexDepth = indexDepth % (numLine*numCol);
          //reset
          if(indexDepth == 0){
            startFrame = false;
            Serial.println();  
          }
        }
      }
    }
  }
}
