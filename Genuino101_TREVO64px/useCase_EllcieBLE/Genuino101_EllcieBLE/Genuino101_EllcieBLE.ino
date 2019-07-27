#include <CurieBLE.h>

BLEPeripheral blePeripheral;  // BLE Peripheral Device
BLEService sensor("838f7fdd-4c42-405f-b8d4-83a698cce2e0");
BLECharacteristic streamSensor("a864bb58-1b21-4b89-8f5a-6947341abbf0", BLERead | BLENotify, 18);
BLEService control("00ff0000-fd7a-4c87-6373-712060e11c1e");
BLECharacteristic startStream("00ff0001-fd7a-4c87-6373-712060e11c1e", BLEWrite, 3);

uint8_t depthMatrix[64] = {0};
bool streamStarted = false;

void updateMatrix(){ 
  uint8_t randum;
  for(int i = 0; i < 64; ++i){ 
    randum = random(255);
    depthMatrix[i] = randum;
  }
}

void sendMatrix(int i){ 
  Serial.print("SENDING ");Serial.println(i);
  uint8_t toSend[18] = {0};
  toSend[0] = 0x50;
  toSend[1] = 0x10;
  for(int j = 0; j < 16; ++j){ 
    toSend[2+j] = depthMatrix[(i*16)+j];
  }
  streamSensor.setValue(toSend, 18);
  delay(250);
}

void validateStratStream(){ 
  if(startStream.written()){ 
    const unsigned char * BLEin = startStream.value();
    if(/*(uint8_t)BLEin[0] == 0xBB &&*/ (uint8_t)BLEin[1] == 0xDF && (uint8_t)BLEin[2] == 0x50){ 
       streamStarted = true;
       Serial.println("STREAM STARTED");
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  //BLE set up
  blePeripheral.setLocalName("ARTEFACTEST");
  blePeripheral.setAdvertisedServiceUuid(sensor.uuid());
  blePeripheral.addAttribute(sensor);
  blePeripheral.addAttribute(streamSensor);
  blePeripheral.addAttribute(control);
  blePeripheral.addAttribute(startStream);
  blePeripheral.begin(); 
  delay(80);

  //DEBUG
  Serial.begin(115200);
  delay(80);
}

void loop() {
  // put your main code here, to run repeatedly:
  BLECentral central = blePeripheral.central();
  if(central){ 
    while(central.connected()){ 
      if(streamStarted == true){ 
        updateMatrix();
        for(int i = 0; i < 4; ++i){ 
          sendMatrix(i);
        }
        
      }else{ 
        validateStratStream();
      }
    }
  }
}
