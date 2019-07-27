#include <SSIManager.h>

#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService glassesService("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc");
BLECharacteristic trame("b60ba360-5c16-4ddb-bdb2-d134fa42bfbc", BLERead | BLENotify, 20);

enum stateSSI{ 
  SSI_SENDING_QUERY = 'q',
  SSI_WAITING_QUERY_RSP = 'a',
  SSI_SENDING_CONFIG = 'g',
  SSI_WAITING_CONFIG_RSP = 'x',
  SSI_SENDING_OBSERVER = 'o',
  SSI_WAITING_OBSERVER_RSP = 'y',
  SSI_PULLING = 'm'
};

uint8_t frameSSI[SSI_FRAME_MAX_LENGTH] = {0}; //0xFE Len ~Len Addr Cmd Data Crc
int indexFrameSSI = 0;
uint8_t * depth_array = NULL;
int indexDepth = 0;
int numLine = -1;
int numCol = -1;
char currentState = SSI_SENDING_QUERY;

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
  }
}

void validateQueryResponse(){ 
  uint8_t notLen = ~frameSSI[2];
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == notLen) && (frameSSI[1] == 0x0F) && (frameSSI[4] == 0x61) /*&& crc16*/){ 
    //TODO : rest of the frame maybe usefull
    currentState = SSI_SENDING_CONFIG;
    for(int i = 0; i < frameSSI[1]; ++i){ 
      frameSSI[i] = 0;
    }
    indexFrameSSI = 0;
    Serial.println("QUERY RESPONSE VALIDATED");
  }else{ 
    Serial.print("ERROR QUERY RESP ");
    Serial.print(frameSSI[0]);Serial.print(" ");
    Serial.print(frameSSI[1]);Serial.print(" ");Serial.print(frameSSI[2]);Serial.print(" ");
    Serial.println(frameSSI[4]);
  }
}

void validateConfigResponse(){
  uint8_t notLen = ~frameSSI[2]; 
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == notLen) && (frameSSI[1] == 0x11) && (frameSSI[4] == 0x78) /*&& crc16*/){ 
    numLine = (((((uint16_t)frameSSI[9])<<8) & 0xFF00) | (((uint16_t)frameSSI[10]) & 0x00FF));
    numCol = (((((uint16_t)frameSSI[13])<<8) & 0xFF00) | (((uint16_t)frameSSI[14]) & 0x00FF));
    if(numLine > 0 && numCol > 0){ 
      if(depth_array != NULL){
        free(depth_array);  
      }
      depth_array = (uint8_t*)calloc(numLine*numCol, sizeof(uint8_t));
      
      currentState = SSI_SENDING_OBSERVER;
      for(int i = 0; i < frameSSI[1]; ++i){ 
        frameSSI[i] = 0;
      }
      indexFrameSSI = 0;
      Serial.println("CONFIG RESPONSE VALIDATED");
    }else{ 
      Serial.print("ERROR ALLOC ");
      Serial.print(numLine);Serial.print(" ");Serial.println(numCol);
    }
  }else{ 
    Serial.print("ERROR CONFIG RESP ");
    Serial.print(frameSSI[0]);Serial.print(" ");
    Serial.print(frameSSI[1]);Serial.print(" ");Serial.print(frameSSI[2]);Serial.print(" ");
    Serial.println(frameSSI[4]);
  }
}

void validateObserverResponse(){ 
  uint8_t notLen = ~frameSSI[2];
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == notLen) && (frameSSI[1] == 0x08) && (frameSSI[4] == 0x79) /*&& crc16*/){ 
    currentState = SSI_PULLING;
    for(int i = 0; i < frameSSI[1]; ++i){ 
      frameSSI[i] = 0;
    }
    indexFrameSSI = 0;
    Serial.println("OBSERVER CREATE VALIDATED");
  }else{ 
    Serial.print("ERROR OBSERVER CREATE ");
    Serial.print(frameSSI[0]);Serial.print(" ");
    Serial.print(frameSSI[1]);Serial.print(" ");Serial.print(frameSSI[2]);Serial.print(" ");
    Serial.println(frameSSI[4]);
  }
}

void parseManyData(){ 
  uint8_t notLen = ~frameSSI[2];
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == notLen) && (frameSSI[1] == 0x49) && (frameSSI[4] == 0x6D) /*&& crc16 && idSensor*/){ 
    if(depth_array != NULL){ 
      int lenMatrix = numLine*numCol;
      for(int i = 0; i < lenMatrix; ++i){ 
        depth_array[i] = frameSSI[i+7];
      }
      for(int j = 0; j < numLine; ++j){ 
        sendDepthArrayLine(j);
      }
      
      for(int i = 0; i < frameSSI[1]; ++i){ 
        frameSSI[i] = 0;
      }
      indexFrameSSI = 0;
      Serial.println("MANY DATA READ AND SENT");
    }
  }else{ 
    Serial.print("ERROR MANY DATA ");
    Serial.print(frameSSI[0]);Serial.print(" ");
    Serial.print(frameSSI[1]);Serial.print(" ");Serial.print(frameSSI[2]);Serial.print(" ");
    Serial.println(frameSSI[4]);
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
}

void loop() {
  BLECentral central = blePeripheral.central();
  
  uint8_t recv;
  uint8_t lenFrameQR = 7;
  uint8_t toSendQR[7] = {0};
  uint8_t lenFrameCF = 9;
  uint8_t toSendCF[9] = {0};
  uint8_t lenFrameOB = 18;
  uint8_t toSendOB[18] = {0};
  uint16_t crcFrame = 0;
  
  switch(currentState){ 
    case SSI_SENDING_QUERY :
      //0x FE 07 F8 '?' 'q' crc16 = 7 bytes
      toSendQR[0] = 0xFE;
      toSendQR[1] = 0x07;
      toSendQR[2] = 0xF8;
      toSendQR[3] = 0x3F;
      toSendQR[4] = 0x71;
      crcFrame = ssi_fnCRC16(toSendQR, lenFrameQR);
      toSendQR[5] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSendQR[6] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0; i < lenFrameQR; ++i){ 
        Serial1.write(toSendQR[i]);
      }
      Serial.println("SENT QUERY");
      currentState = SSI_WAITING_QUERY_RSP;
      break;
    case SSI_WAITING_QUERY_RSP :
      if(Serial1.available() > 0){ 
        recv = (uint8_t)Serial1.read();
        frameSSI[indexFrameSSI] = recv;
        indexFrameSSI++;
      }
      if(indexFrameSSI >= 15){ 
        validateQueryResponse();
      }
      break;
    case SSI_SENDING_CONFIG :
      //0x FE 09 F6 00 'g' 0050 crc16 = 9 bytes
      toSendCF[0] = 0xFE;
      toSendCF[1] = 0x09;
      toSendCF[2] = 0xF6;
      toSendCF[3] = 0x00;
      toSendCF[4] = 0x67;
      toSendCF[5] = 0x00;
      toSendCF[6] = 0x50;
      crcFrame = ssi_fnCRC16(toSendCF, lenFrameCF);
      toSendCF[7] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSendCF[8] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0; i < lenFrameCF; ++i){ 
        Serial1.write(toSendCF[i]);
      }
      Serial.println("SENT GET CONFIG");
      currentState = SSI_WAITING_CONFIG_RSP;
      break;
    case SSI_WAITING_CONFIG_RSP :
      if(Serial1.available() > 0){ 
        recv = (uint8_t)Serial1.read();
        frameSSI[indexFrameSSI] = recv;
        indexFrameSSI++;
      }
      if(indexFrameSSI >= 17){ 
        validateConfigResponse();
      }
      break;
    case SSI_SENDING_OBSERVER :
      //0x FE 12 ED 00 'o'/6F 0000 01 00 40 00000000 0050 crc16 = 18 bytes
      toSendOB[0] = 0xFE;
      toSendOB[1] = 0x12;
      toSendOB[2] = 0xED;
      toSendOB[3] = 0x00;
      toSendOB[4] = 0x6F;
      toSendOB[5] = 0x00;
      toSendOB[6] = 0x00;
      toSendOB[7] = 0x01;
      toSendOB[8] = 0x00;
      toSendOB[9] = 0x40;
      toSendOB[10] = 0x00;
      toSendOB[11] = 0x00;
      toSendOB[12] = 0x00;
      toSendOB[13] = 0x00;
      toSendOB[14] = 0x00;
      toSendOB[15] = 0x50;
      crcFrame = ssi_fnCRC16(toSendOB, lenFrameOB);
      toSendOB[16] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSendOB[17] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0; i < lenFrameOB; ++i){ 
        Serial1.write(toSendOB[i]);
      }
      Serial.println("SENT OBSERVER CREATE");
      currentState = SSI_WAITING_OBSERVER_RSP;
      break;
    case SSI_WAITING_OBSERVER_RSP :
      if(Serial1.available() > 0){ 
        recv = (uint8_t)Serial1.read();
        frameSSI[indexFrameSSI] = recv;
        indexFrameSSI++;
      }
      if(indexFrameSSI >= 8){ 
        validateObserverResponse();
      }
      break;
    case SSI_PULLING :
      if(Serial1.available() > 0){ 
        recv = (uint8_t)Serial1.read();
        frameSSI[indexFrameSSI] = recv;
        indexFrameSSI++;
      }
      if(indexFrameSSI >= 73){ 
        if(central && central.connected()){ 
          parseManyData();
        }else{ 
          for(int i = 0; i < frameSSI[1]; ++i){ 
            frameSSI[i] = 0;
          }
          indexFrameSSI = 0;
        }
      }
      break;
    default :
      Serial.print("UNKOWN CMD ");Serial.println(currentState);
      break;
  }
}
