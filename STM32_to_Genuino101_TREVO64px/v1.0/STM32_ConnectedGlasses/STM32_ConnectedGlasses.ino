HardwareSerial Serial1(PA10, PA9); //rx, tx from sensor
HardwareSerial Serial4(PA1, PA0); //rx, tx from sensor
int NUMLINE = 8;
int NUMCOL = 8;
int FRAMETOSKIP = 10;
int frameSkipped = 0;
int depth_array[64] = {0};
byte frame[137] = {0}; //1 byte header + 64*2 bytes of data + 8 bytes of CRC32
int indexFrame = 0;
int indexCRC = 0;
bool hasDepthMapToSend = false;
bool paddingEnded = false;
bool setupDone = false;

//Parsing the trame to the depth matrix, distances between 0 to 5000mm mapped to range 1-254
void parseEvo(){
  byte header;
  uint16_t distance;
  header = frame[0];
  if(header == 0x11){
    for(int i = 1; i < 65; ++i){
      distance = ((frame[2*i-1] & 0x1F) << 7) | (frame[2*i] & 0x7F);
      // ou distance = (frame[2*i-1]<<8) + frame[2*i];
      distance = (distance > 5000) ? 5000 : distance;
      distance = map(distance, 0, 5000, 1, 254);
      depth_array[i-1] = distance;
    }
    //crc32 only on the 4 lower bits of each
    /*for(int j = 129 ; j < 137; ++j){
      uint8_t crc = frame[j] (& 0x0F);
    }*/
    hasDepthMapToSend = true;
   }  
}

void sendSSIDepth(){
  Serial4.write(0);
  delay(5);
  for(int i = 0; i < 64; ++i){
    Serial4.write(depth_array[i]);
    delay(5);
  }
  hasDepthMapToSend = false;
}

void sendSSIDepthDEBUG(){
  Serial.print(0);Serial.print(" ");
  delay(5);
  for(int i = 0; i < 64; ++i){
    Serial.print(depth_array[i]);Serial.print(" ");
    delay(5);
  }
  Serial.println();
  hasDepthMapToSend = false;
}

void setup() { 
  /*TERA EVO 64px UART*/
  Serial1.begin(3000000);
  delay(80);
  Serial1.write("\x00\x11\x02\x4C"); //distance mode
  delay(80);
  /*Serial1.write("\x00\x21\x02\xB5"); //fast mode
  delay(100);*/

  //Serial communication with main board
  //Set the baud rate of your needs
  Serial4.begin(115200);
  delay(80);
  while(!Serial4){;}

  Serial.begin(115200);
  delay(100);
}

void loop() {
  byte recv;
  if(setupDone == false){ //TRANSMIT INITIAL DATA : size of the depth matrix
    Serial4.write(255);
    delay(5);
    Serial4.write(NUMLINE);
    delay(5);
    Serial4.write(NUMCOL);
    delay(5);
    setupDone = true;
  }else{ //SENDING THE DEPTH MATRIX TO MAIN BOARD
    if(indexFrame == 129 && indexCRC == 8){
      parseEvo();
      if(hasDepthMapToSend == true && frameSkipped == 0){
         sendSSIDepth();
      }
      frameSkipped++;
      frameSkipped = frameSkipped % FRAMETOSKIP;  
      indexFrame = 0; 
      indexCRC = 0;
      paddingEnded = false; 
    }else{ //READING DATA FROM SENSOR 
      while(Serial1.available()>0){
        recv = byte(Serial1.read());
        if(indexFrame == 0){ //HEADER
          if(recv == 0x11){
            frame[indexFrame] = recv;
            indexFrame++;
          }
        }
        else if(indexFrame > 0 && indexFrame < 129){ //DISTANCE DATA
          frame[indexFrame] = recv;
          indexFrame++;
        }
        else{ //PADDING AND CRC32
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
}
