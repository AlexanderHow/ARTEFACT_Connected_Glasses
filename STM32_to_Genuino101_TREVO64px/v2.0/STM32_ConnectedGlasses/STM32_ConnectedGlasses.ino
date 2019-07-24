#define NUMLINE 8
#define NUMCOL 8
#define FRAMETOSKIP 10

#define SSI_FRAME_MAX_PAYLOAD_SIZE 255
#define SSI_FRAME_HEADER_SIZE 3
#define SSI_FRAME_FOOTER_SIZE 2
#define SSI_FRAME_OVERHEAD_SIZE (SSI_FRAME_HEADER_SIZE + SSI_FRAME_FOOTER_SIZE)
#define SSI_FRAME_MAX_LENGTH (SSI_FRAME_MAX_PAYLOAD_SIZE + SSI_FRAME_OVERHEAD_SIZE)

#define SSI_FRAME_SOF           0xFE
#define SSI_FRAME_SOF_INDEX     0
#define SSI_FRAME_LEN_INDEX     1
#define SSI_FRAME_NOT_LEN_INDEX 2 
#define SSI_FRAME_ADDR_INDEX    3 
#define SSI_FRAME_CMD_INDEX     4
#define SSI_FRAME_SOP_INDEX     5

HardwareSerial Serial1(PA10, PA9); //rx, tx from sensor
HardwareSerial Serial4(PA1, PA0); //rx, tx to main board

enum ssi_commands_defition
{
   SSI_WILDCARD = '?',
   SSI_QUERY = 'q',
   SSI_QUERY_RSP = 'a',
   SSI_DISCOVER_SENSORS = 'c',
   SSI_DISCOVER_REPLY = 'n',
   SSI_GET_CONFIGURATION_DATA = 'g',
   SSI_CONFIGURATION_DATA_RSP = 'x',
   SSI_SENSOR_DATA_RSP = 'd',
   SSI_SENSOR_MANY_DATA_RSP = 'm',
   SSI_CREATE_SENSOR_OBSERVER = 'o',
   SSI_OBSERVER_CREATED = 'y',
   SSI_KILL_SENSOR_OSERVER = 'k'
}

uint8_t depthMatrix[64] = {0};
byte frameSensor[137] = {0}; //1 byte header + 64*2 bytes of data + 8 bytes of CRC32
int indexFrameSensor = 0;
int totalLenFrame = 0;

byte frameSSI[SSI_FRAME_MAX_LENGTH] = {0}; //0xFE Len ~Len Addr Cmd Data Crc
int indexFrameSSI = 0;

bool observerCreated = false;

bool checkCrcSSI(){ 
  //TODO
  return true;
}

void resetFrameSSI(){ 
  for(int i = 0, i < SSI_FRAME_MAX_LENGTH; ++i){ 
    frameSSI[i] = 0;
  }
}

////////////////
//PARSER SSI ///
////////////////

void parseSSI(){ 
  if(frameSSI[SSI_FRAME_SOF_INDEX] == 0xFE &&
     frameSSI[SSI_FRAME_LEN_INDEX] == (~frameSSI[SSI_FRAME_NOT_LEN_INDEX]) &&
     checkCrcSSI() == true){ 

      switch(frameSSI[SSI_FRAME_CMD_INDEX]){ 
        case SSI_QUERY:
          sendSSI_queryRsp();
          break;
        case SSI_DISCOVER_SENSORS:
          sendSSI(SSI_DISCOVER_REPLY);
          break;
        case SSI_GET_CONFIGURATION_DATA:
          sendSSI(SSI_CONFIGURATION_DATA_RSP);
          break;
        case SSI_CREATE_SENSOR_OBSERVER:
          observerCreated = true;
          sendSSI(SSI_OBSERVER_CREATED);
          break;
        case SSI_KILL_SENSOR_OSERVER:
          observerCreated = false;
          break;
        default:
          break;
      }

      resetFrameSSI();
  }
}

///////////////////
//SEND SSI FRAME///
///////////////////

//reponse constante 0xFE 0x0F 0xF0 0x00 'a' 0x0100 FF00 0000 0000 0x5170
void sendSSI_queryRsp(){ 
  while(Serial4.availableForWrite() < 15){ 
    delay(2);
  }
  Serial4.write("\xFE\x0F\xF0\x00\x61\x01\x00\xFF\x00\x00\x00\x00\x00\x51\x70");
}

// 0x FE 2D D2 00 'n' FFFF "_Tera_evo_64_px_" "depth___" 01 00 000000 0000FF crc16 = 43 bytes
void sendSSI_discoverReply(){ 
  byte toSend[43] = {0};
  int sensorId = 0xFFFF;
  uint8_t type = 0x01;
  uint8_t scaler = 0x00;
  uint8_t mini[4] = {0x00, 0x00, 0x00, 0x00};
  uint8_t maxi[4] = {0x00, 0x00, 0x00, 0xFF}; 
  char description[16] = {'_','T','e','r','a','_','e','v','o','_','6','4','_','p','x','_'};
  char unit = {'d','e','p','t','h','_','_','_'};
  uint8_t lenFrame = (uint8_t)sizeof(toSend);

  toSend[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
  toSend[SSI_FRAME_LEN_INDEX] = lenFrame;
  toSend[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
  toSend[SSI_FRAME_ADDR_INDEX] = 0x00;
  toSend[SSI_FRAME_CMD_INDEX] = byte('n');

  toSend[5] = (uint8_t)((sensorId & 0xFF00)>>8);
  toSend[6] = (uint8_t)(sensorId & 0x00FF)
  for(int i = 7; i < 23; ++i){ 
    toSend[i] = byte(description[i-7]); 
  }
  for(int j = 23; j < 31; ++j){ 
    toSend[j] = byte(unit[j-23]); 
  }
  toSend[31] = type;
  toSend[32] = scaler;
  for(int k = 33; k < 37; ++k){ 
    toSend[k] = mini[k-33]; 
  }
  for(int l = 37; l < 41; ++l){ 
    toSend[l] = maxi[l-37]; 
  }
  addCRC16(toSend, lenFrame, SSI_FRAME_SOP_INDEX);

  while(Serial4.availableForWrite() < 43){ 
    delay(2);
  }
  Serial4.write(toSend, sizeof(toSend));
}

void sendSSI_configurationRsp(){ 
  
}

void sendSSI_observerCreated(){ 
  
}

void sendSSI_manyData(){ 
  
}

void setup() { 
  /*TERA EVO 64px UART*/
  Serial1.begin(3000000);
  while(!Serial1){;}
  Serial1.write("\x00\x11\x02\x4C"); //distance mode
  delay(50);

  //Serial communication with main board
  //Set the baud rate of your needs
  Serial4.begin(115200);
  while(!Serial4){;}
  delay(50);
}

void loop() {
  
}
