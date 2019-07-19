HardwareSerial Serial1(PA10, PA9); //rx, tx from sensor
HardwareSerial Serial4(PA1, PA0); //rx, tx to main board

int NUMLINE = 8;
int NUMCOL = 8;
int FRAMETOSKIP = 10;

int matrix[64] = {0};
int indexDepth = 0;
int frameSkipped = 0;
byte crc32[8] = {0};
int indexFrame = 0;
int indexCRC = 0;
byte highRecvByte = 0;
byte lowRecvByte = 0;
bool paddingEnded = false;
bool setupDone = false;

void sendInitialData(){
  if(Serial4){ 
    Serial4.write(255);
    delay(5);
    Serial4.write(NUMLINE);
    delay(5);
    Serial4.write(NUMCOL);
    delay(5);
    setupDone = true;
  }
}

int getDistance(){ 
  int distance = ((highRecvByte & 0x1F) << 7) | (lowRecvByte & 0x7F);
  distance = (distance > 5000) ? 5000 : distance;
  distance = map(distance, 0, 5000, 1, 254);
  return distance;
}

void headerAndDepthParser(byte recv){ 
  if( recv == 17){ //start of the depth data stream
    indexFrame = 1;
    if(frameSkipped == 0){
      Serial4.write(0); //mark skip
      delay(5);         //mark skip
    }
  }

  if(indexFrame > 0){ //stream of data has strated, we got the 0x11 header
    if((indexFrame%2) == 1){ //the high byte
      highRecvByte = recv;
    }else{  //the low byte
      lowRecvByte = recv;
      int d = getDistance();
      if(frameSkipped == 0){ 
        matrix[indexDepth]=d; 
        indexDepth++;
        Serial4.write(d);//mark skip
        delay(5);        //mark skip
      }
    }
    indexFrame++;
  }
}

void paddingAndCrcParser(byte recv){ 
  if(paddingEnded == true){ //crc
    crc32[indexCRC] = recv;
    indexCRC++;
    if(indexCRC > 7){ //end of crc32 and reset of the state of the stream reading process
      indexCRC = 0;
      indexFrame = 0;
      paddingEnded = false;
      //mark incr frame skipped
      frameSkipped++;
      frameSkipped=frameSkipped%FRAMETOSKIP;
      //TODO : check crc, if so replace in headerAndDepthParser "Serila4.write(d);" by "matrix[indexDepth]=d; indexDepth++;" and send all the matrix on Serial4 if the crc is ok
      indexDepth = 0;
    }
  }else{ 
    //padding = 0x80
    if(recv != 128){ 
      paddingEnded = true;
      crc32[indexCRC] = recv;
      indexCRC++;
    }
  }
}

void setup() { 
  /*TERA EVO 64px UART*/
  Serial1.begin(3000000);
  while(!Serial1){;}
  Serial1.write("\x00\x11\x02\x4C"); //distance mode
  delay(50);
  Serial1.write("\x00\x21\x01\xBC"); //close range mode
  delay(50);

  //Serial communication with main board
  //Set the baud rate of your needs, would need to be 3MBps
  Serial4.begin(115200);
  while(!Serial4){;}
  delay(50);
}

void loop() {
  byte recv;
  if(setupDone == false){ //TRANSMIT INITIAL DATA : size of the depth matrix
    sendInitialData();
  }else{ //SENDING THE DEPTH MATRIX TO MAIN BOARD
    while(Serial1.available()>0){ 
      recv = byte(Serial1.read());
      if(indexFrame < 129){ //HEADER + DEPTH DATA
        headerAndDepthParser(recv);
      }else{ //PADDING + CRC32
        paddingAndCrcParser(recv);
      }
    }   
  }
}
