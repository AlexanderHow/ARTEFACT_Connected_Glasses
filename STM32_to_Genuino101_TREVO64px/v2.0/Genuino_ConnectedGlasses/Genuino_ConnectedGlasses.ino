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
stateSSI currentState = SSI_SENDING_QUERY;

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

void validateQueryResponse(){ 
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == ~frameSSI[2]) && (frameSSI[1] == 0x0F) && (frameSSI[4] == 0x61) /*&& crc16*/){ 
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
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == ~frameSSI[2]) && (frameSSI[1] == 0x11) && (frameSSI[4] == 0x78) /*&& crc16*/){ 
    numLine = (((((uint16_t)frameSSI[9])<<8) & 0xFF00) | (((uint16_t)frame[10]) & 0x00FF));
    numCol = (((((uint16_t)frameSSI[13])<<8) & 0xFF00) | (((uint16_t)frame[14]) & 0x00FF));
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
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == ~frameSSI[2]) && (frameSSI[1] == 0x08) && (frameSSI[4] == 0x79) /*&& crc16*/){ 
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
  if((frameSSI[0] == 0xFE) && (frameSSI[1] == ~frameSSI[2]) && (frameSSI[1] == 0x49) && (frameSSI[4] == 0x6D) /*&& crc16 && idSensor*/){ 
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
  uint8_t recv;
  BLECentral central = blePeripheral.central();
  switch(currentState){ 
    case SSI_SENDING_QUERY :
      //0x FE 07 F8 '?' 'q' crc16 = 7 bytes
      uint8_t lenFrame = 7;
      uint8_t toSend[7] = {0};
      toSend[0] = 0xFE;
      toSend[1] = 0x07;
      toSend[2] = 0xF8;
      toSend[3] = 0x3F;
      toSend[4] = 0x71;
      uint16_t crcFrame = ssi_fnCRC16(toSend, lenFrame);
      toSend[5] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSend[6] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0, i < lenFrame; ++i){ 
        Serial1.write(toSend[i]);
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
      uint8_t lenFrame = 9;
      uint8_t toSend[9] = {0};
      toSend[0] = 0xFE;
      toSend[1] = 0x09;
      toSend[2] = 0xF6;
      toSend[3] = 0x00;
      toSend[4] = 0x67;
      toSend[5] = 0x00;
      toSend[6] = 0x50;
      uint16_t crcFrame = ssi_fnCRC16(toSend, lenFrame);
      toSend[7] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSend[8] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0, i < lenFrame; ++i){ 
        Serial1.write(toSend[i]);
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
      uint8_t lenFrame = 18;
      uint8_t toSend[18] = {0};
      toSend[0] = 0xFE;
      toSend[1] = 0x12;
      toSend[2] = 0xED;
      toSend[3] = 0x00;
      toSend[4] = 0x6F;
      toSend[5] = 0x00;
      toSend[6] = 0x00;
      toSend[7] = 0x01;
      toSend[8] = 0x00;
      toSend[9] = 0x40;
      toSend[10] = 0x00;
      toSend[11] = 0x00;
      toSend[12] = 0x00;
      toSend[13] = 0x00;
      toSend[14] = 0x00;
      toSend[15] = 0x50;
      uint16_t crcFrame = ssi_fnCRC16(toSend, lenFrame);
      toSend[16] = (uint8_t)((crcFrame >> 8) & 0xFF);
      toSend[17] = (uint8_t)(crcFrame & 0xFF);
      for(int i = 0, i < lenFrame; ++i){ 
        Serial1.write(toSend[i]);
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
        parseManyData();
      }
      break;
     default :
      Serial.print("UNKOWN CMD ");Serial.println(currentState);
      break;
  }
}
