#include <SSIManager.h>

HardwareSerial Serial1(PA10, PA9); //rx, tx from sensor
HardwareSerial Serial4(PA1, PA0); //rx, tx to main board

uint8_t depthMatrix[64] = {0};
uint8_t frameSensor[137] = {0}; //1 byte header + 64*2 bytes of data + 8 bytes of CRC32
int indexFrameSensor = 0;
int totalLenFrame = 0; // len - 137 = X byte of padding
bool paddingEnded = false;
uint8_t * toSendMD;

uint8_t frameSSI[SSI_FRAME_MAX_LENGTH] = {0}; //0xFE Len ~Len Addr Cmd Data Crc
int indexFrameSSI = 0;
int lenFrameSSI = 0;

bool observerCreated = false;
bool hasDepthsToSend = false;

void resetFrameSSI(){
  indexFrameSSI = 0;
  lenFrameSSI = 0;
  for(int i = 0; i < SSI_FRAME_MAX_LENGTH; ++i){ 
    frameSSI[i] = 0;
  }
}

/////////////////////
//PARSE SENSOR DATA//
/////////////////////
//Parsing the trame to the depth matrix, distances between 0 to 5000mm mapped to range 0-255
void parseSensor(){
  uint8_t header;
  uint16_t distance;
  header = frameSensor[0];
  if(header == 0x11){
    for(int i = 1; i < 65; ++i){
      distance = ((frameSensor[2*i-1] & 0x1F) << 7) | (frameSensor[2*i] & 0x7F);
      distance = (distance > 5000) ? 5000 : distance;
      distance = map(distance, 0, 5000, 0, 255);
      depthMatrix[i-1] = distance;
    }
    //crc32 only on the 4 lower bits of each (warning : frameSensor doesn't contain the padding (0x80 x (lenFrame - 137))
    /*for(int j = 129 ; j < 137; ++j){
      uint8_t crc = frame[j] (& 0x0F);
    }*/
    //Serial.println("SENSOR FRAME PARSED SUCCESFULLY");
    hasDepthsToSend = true;
  }  
}

////////////////
//PARSER SSI ///
////////////////
void parseSSI(){ 
  uint8_t notLen = ~frameSSI[SSI_FRAME_NOT_LEN_INDEX];
  if(frameSSI[SSI_FRAME_SOF_INDEX] == 0xFE &&
     frameSSI[SSI_FRAME_LEN_INDEX] == notLen /*&&
     ssi_frame_check_crc(frameSSI, frameSSI[SSI_FRAME_LEN_INDEX]) == 0*/){ 
      uint8_t lenFrame = 0;
      uint8_t * toSend;
      switch(frameSSI[SSI_FRAME_CMD_INDEX]){ 
        case SSI_QUERY:
          lenFrame = 15;
          SSI_queryRsp(&toSend, lenFrame);
          break;
        case SSI_DISCOVER_SENSORS:
          lenFrame = 43;
          SSI_discoverReply(&toSend, lenFrame);
          break;
        case SSI_GET_CONFIGURATION_DATA:
          //TODO : care about id sensor in the cmd even if only one sensor here ?
          lenFrame = 17;
          SSI_configurationRsp(&toSend, lenFrame);
          break;
        case SSI_CREATE_SENSOR_OBSERVER:
          observerCreated = true;
          lenFrame = 8;
          SSI_observerCreated(&toSend, lenFrame);
          break;
        case SSI_KILL_SENSOR_OSERVER:
          observerCreated = false;
          break;
        default:
          break;
      }
      if(lenFrame > 0 && toSend != NULL){ 
        for(int i = 0; i < lenFrame; ++i){ 
          Serial4.write(toSend[i]);
        }
      }
      lenFrame = 0;
      if(toSend != NULL){free(toSend);}
      resetFrameSSI();
  }
}

void setup() { 
  /*TERA EVO 64px UART*/
  Serial1.begin(3000000);
  while(!Serial1){;}
  Serial1.write("\x00\x11\x02\x4C"); //distance mode
  delay(50);

  Serial.begin(115200);
  delay(50);

  //Serial communication with main board
  //Set the baud rate of your needs
  Serial4.begin(115200);
  while(!Serial4){;}
  delay(50);
}

void loop() {
  //read ssi cmd
  uint8_t recvSSI;
  while(Serial4.available() > 0){ 
    recvSSI = (uint8_t)Serial4.read();
    if(recvSSI == 0xFE){ 
      indexFrameSSI = 0;
      frameSSI[indexFrameSSI] = recvSSI;
      indexFrameSSI++;
    }else{ 
      if(indexFrameSSI == 1){ 
        lenFrameSSI = recvSSI;
        frameSSI[indexFrameSSI] = recvSSI;
        indexFrameSSI++;
      }else{ 
        frameSSI[indexFrameSSI] = recvSSI;
        indexFrameSSI++;
        if(indexFrameSSI >= lenFrameSSI){ 
          parseSSI();
          break;
        }
      }
    }
  }

  uint8_t recvSensor;
  //READING DATA FROM SENSOR 
  while(Serial1.available()>0){
    recvSensor = (uint8_t)Serial1.read();
    if(indexFrameSensor == 0){ //HEADER
      if(recvSensor == 0x11){
        //Serial.println("START SENSOR FRAME");
        frameSensor[indexFrameSensor] = recvSensor;
        indexFrameSensor++;
        totalLenFrame++;
      }
    }
    else if(indexFrameSensor > 0 && indexFrameSensor < 129){ //DISTANCE DATA
      frameSensor[indexFrameSensor] = recvSensor;
      indexFrameSensor++;
      totalLenFrame++;
    }
    else{ //PADDING AND CRC32
      if(paddingEnded == true){
        if(indexFrameSensor < 137){
          frameSensor[indexFrameSensor] = recvSensor;
          indexFrameSensor++;
          totalLenFrame++;
        }else{
          //Serial.println("END SENSOR FRAME");
          break;
        }
      }else{
        if(recvSensor != 128){ //0x80 = padding
          frameSensor[indexFrameSensor] = recvSensor;
          indexFrameSensor++;
          totalLenFrame++;
          paddingEnded = true;
        }else{ 
          totalLenFrame++;
        }
      }
    }
  }

  if(indexFrameSensor >= 137){
    parseSensor();
    indexFrameSensor = 0;
    totalLenFrame = 0;
  }

  //send ssi cmd 'm'
  if(observerCreated == true && hasDepthsToSend == true){ //print here
    SSI_manyData(&toSendMD, 73, depthMatrix, 64);
    if(toSendMD != NULL){ 
      //Serial.println("SENDING SENSOR FRAME");
      for(int i = 0; i < 73; ++i){ 
        Serial4.write(toSendMD[i]);
      }
    }
    hasDepthsToSend = false;
  }
}
