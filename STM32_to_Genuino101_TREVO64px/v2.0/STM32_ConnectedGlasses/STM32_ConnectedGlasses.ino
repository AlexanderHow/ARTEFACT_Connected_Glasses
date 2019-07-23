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

void parseSSI(){ 
  if(frameSSI[SSI_FRAME_SOF_INDEX] == 0xFE &&
     frameSSI[SSI_FRAME_LEN_INDEX] == (~frameSSI[SSI_FRAME_NOT_LEN_INDEX]) &&
     checkCrcSSI() == true){ 

      switch(frameSSI[SSI_FRAME_CMD_INDEX]){ 
        case SSI_QUERY:
          sendSSI(SSI_QUERY_RSP);
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
