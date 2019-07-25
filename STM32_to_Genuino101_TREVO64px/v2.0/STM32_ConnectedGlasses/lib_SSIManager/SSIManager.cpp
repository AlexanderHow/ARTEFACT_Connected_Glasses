/*
  SSIManager.cpp - Library for computing and checking crc16 + generating ssi commands.
  Created by Pierre LeCoz and Alexandre Howard.
  7/25/2019
*/

#include "SSIManager.h"

///////////////////
// Calcul du CRC //
///////////////////
uint16_t ssi_fnCRC16(const uint8_t *ptrFrame, uint16_t len)
{
    // Shift SOF, len and ~len
    ptrFrame += SSI_FRAME_HEADER_SIZE;

    uint16_t crc = 0;

    for(int i = 0; i < (len - SSI_FRAME_OVERHEAD_SIZE); i++)
    {
        crc ^= *ptrFrame++;

        for(int j = 0; j < 8; j++)
        {
            if(crc & 1)
            {
                crc = crc >> 1;
                crc ^= 0xA001;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }

    return crc;
}

///////////////////////////
// Vérification du CRC   //
///////////////////////////
static int ssi_frame_check_crc(const char *frame, uint16_t len)
{
    uint8_t payload_size = 0;
    uint8_t cmd = 0; 
    uint16_t crc_pos = 0;
    uint16_t lo = 0;
    uint16_t hi = 0; 
    uint16_t rcv_crc = 0, calc_crc = 0;
    int ret = -1;

    if (!frame)
    {
        return -1;
    }

    if (len <= SSI_FRAME_NOT_LEN_INDEX)
    {
        return -1;
    }

    cmd = frame[SSI_FRAME_CMD_INDEX];
        
    if(cmd > 'A' && cmd < 'Z')
    {
        // No CRC when command is upper case
        return 0;
    }

    payload_size = frame[SSI_FRAME_LEN_INDEX] - SSI_FRAME_OVERHEAD_SIZE;
    crc_pos = SSI_FRAME_HEADER_SIZE + payload_size;

    if ((crc_pos + 1) >= frame[SSI_FRAME_LEN_INDEX])
    {
        return -1;
    }

    hi = (((uint16_t)frame[crc_pos]) << 8) & 0xFF00;
    lo = (uint16_t)frame[crc_pos + 1] & 0x00FF;

    // Read CRC
    rcv_crc = (hi | lo);
    // Is CRC ok ?
    // Re-Calculate CRC
    calc_crc = ssi_fnCRC16((uint8_t *)frame, (uint16_t)len);

    if (calc_crc - rcv_crc == 0)
    {
        // crc ok
        ret = 0;
    }
    else
    {
        //error
    }
    return ret;
}

///////////////////
//SEND SSI FRAME///
///////////////////
//reponse constante 0xFE 0x0F 0xF0 0x00 'a' 0x0100 FF00 0000 0000 0x5170 = 15 bytes
void SSI_queryRsp(uint8_t ** toSend, uint8_t lenFrame){ 
  if(lenFrame == 15){ 
	free(*toSend);
	*toSend = (uint8_t *)calloc(lenFrame , sizeof(uint8_t));
	if(*toSend == NULL){ 
		return;
	}
    (*toSend)[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
    (*toSend)[SSI_FRAME_LEN_INDEX] = lenFrame;
    (*toSend)[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
    (*toSend)[SSI_FRAME_ADDR_INDEX] = 0x00;
    (*toSend)[SSI_FRAME_CMD_INDEX] = 0x61;
    (*toSend)[5] = 0x01;
    (*toSend)[6] = 0x00;
    (*toSend)[7] = 0xFF;
    (*toSend)[8] = 0x00;
    (*toSend)[9] = 0x00;
    (*toSend)[10] = 0x00;
    (*toSend)[11] = 0x00;
    (*toSend)[12] = 0x00;
    (*toSend)[13] = 0x51;
    (*toSend)[14] = 0x70;
  }
}

// 0x FE 2D D2 00 'n' 0050 "_Tera_evo_64_px_" "depth_1B" 01 00 000000 0000FF crc16 = 43 bytes
void SSI_discoverReply(uint8_t ** toSend, uint8_t lenFrame){ 
  if(lenFrame == 43){ 
	free(*toSend);
	*toSend = (uint8_t *)calloc(lenFrame , sizeof(uint8_t));
	if(*toSend == NULL){ 
		return;
	}
    int sensorId = 0x0050;
    uint8_t type = 0x01;
    uint8_t scaler = 0x00;
    uint8_t mini[4] = {0x00, 0x00, 0x00, 0x00};
    uint8_t maxi[4] = {0x00, 0x00, 0x00, 0xFF}; 
    char description[16] = {'_','T','e','r','a','_','e','v','o','_','6','4','_','p','x','_'};
    char unit[8] = {'d','e','p','t','h','_','1','B'};
  
    (*toSend)[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
    (*toSend)[SSI_FRAME_LEN_INDEX] = lenFrame;
    (*toSend)[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
    (*toSend)[SSI_FRAME_ADDR_INDEX] = 0x00;
    (*toSend)[SSI_FRAME_CMD_INDEX] = 0x6E;
  
    (*toSend)[5] = (uint8_t)((sensorId & 0xFF00)>>8);
    (*toSend)[6] = (uint8_t)(sensorId & 0x00FF);
    for(int i = 7; i < 23; ++i){ 
      (*toSend)[i] = uint8_t(description[i-7]); 
    }
    for(int j = 23; j < 31; ++j){ 
      (*toSend)[j] = uint8_t(unit[j-23]); 
    }
    (*toSend)[31] = type;
    (*toSend)[32] = scaler;
    for(int k = 33; k < 37; ++k){ 
      (*toSend)[k] = mini[k-33]; 
    }
    for(int l = 37; l < 41; ++l){ 
      (*toSend)[l] = maxi[l-37]; 
    }
    uint16_t crcFrame = ssi_fnCRC16(*toSend, lenFrame);
    (*toSend)[41] = (uint8_t)((crcFrame >> 8) & 0xFF);
    (*toSend)[42] = (uint8_t)(crcFrame & 0xFF);    
  }
}

//0x FE 11 EE 00 'x' 0050 12 'L' 0008 12 'C' 0008 crc16 = 17 bytes
void SSI_configurationRsp(uint8_t ** toSend, uint8_t lenFrame){ 
  if(lenFrame == 17){ 
	free(*toSend);
	*toSend = (uint8_t *)calloc(lenFrame , sizeof(uint8_t));
	if(*toSend == NULL){ 
		return;
	}
    int sensorId = 0x0050;
    
    (*toSend)[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
    (*toSend)[SSI_FRAME_LEN_INDEX] = lenFrame;
    (*toSend)[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
    (*toSend)[SSI_FRAME_ADDR_INDEX] = 0x00;
    (*toSend)[SSI_FRAME_CMD_INDEX] = 0x78;
    (*toSend)[5] = (uint8_t)((sensorId & 0xFF00)>>8);
    (*toSend)[6] = (uint8_t)(sensorId & 0x00FF);
  
    (*toSend)[7] = 0x12;
    (*toSend)[8] = 0x4C;
    int numberOfLine = NUMLINE;
    (*toSend)[9] = (uint8_t)((numberOfLine >> 8) & 0xFF);
    (*toSend)[10] = (uint8_t)(numberOfLine & 0xFF);
  
    (*toSend)[11] = 0x12;
    (*toSend)[12] = 0x43;
    int numberOfColumn = NUMCOL;
    (*toSend)[13] = (uint8_t)((numberOfColumn >> 8) & 0xFF);
    (*toSend)[14] = (uint8_t)(numberOfColumn & 0xFF);
  
    uint16_t crcFrame = ssi_fnCRC16(*toSend, lenFrame);
    (*toSend)[15] = (uint8_t)((crcFrame >> 8) & 0xFF);
    (*toSend)[16] = (uint8_t)(crcFrame & 0xFF);   
  }
}

//0x FE 08 F7 00 'y' 00 crc16 = 8 bytes 
void SSI_observerCreated(uint8_t ** toSend, uint8_t lenFrame){ 
  if(lenFrame == 8){ 
	free(*toSend);
	*toSend = (uint8_t *)calloc(lenFrame , sizeof(uint8_t));
	if(*toSend == NULL){ 
		return;
	}
    (*toSend)[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
    (*toSend)[SSI_FRAME_LEN_INDEX] = lenFrame;
    (*toSend)[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
    (*toSend)[SSI_FRAME_ADDR_INDEX] = 0x00;
    (*toSend)[SSI_FRAME_CMD_INDEX] = 0x79;
    (*toSend)[5] = 0x00;
  
    uint16_t crcFrame = ssi_fnCRC16(*toSend, lenFrame);
    (*toSend)[6] = (uint8_t)((crcFrame >> 8) & 0xFF);
    (*toSend)[7] = (uint8_t)(crcFrame & 0xFF);
  }
}

//0x FE 49 B6 00 'm' 0050 64_data_of_1_byte crc16 = 73 bytes
void SSI_manyData(uint8_t ** toSend, uint8_t lenFrame, uint8_t * depths, int lenDepthArray){ 
  if(lenFrame == 73 && lenDepthArray == 64){ 
	free(*toSend);
	*toSend = (uint8_t *)calloc(lenFrame , sizeof(uint8_t));
	if(*toSend == NULL){ 
		return;
	}
    int sensorId = 0x0050;
    
    (*toSend)[SSI_FRAME_SOF_INDEX] = SSI_FRAME_SOF;
    (*toSend)[SSI_FRAME_LEN_INDEX] = lenFrame;
    (*toSend)[SSI_FRAME_NOT_LEN_INDEX] = ~lenFrame;
    (*toSend)[SSI_FRAME_ADDR_INDEX] = 0x00;
    (*toSend)[SSI_FRAME_CMD_INDEX] = 0x6D;
    (*toSend)[5] = (uint8_t)((sensorId & 0xFF00)>>8);
    (*toSend)[6] = (uint8_t)(sensorId & 0x00FF);
  
    for(int i = 0; i < lenDepthArray; ++i){ 
      (*toSend)[i+7] = depths[i];
    }
  
    uint16_t crcFrame = ssi_fnCRC16(*toSend, lenFrame);
    (*toSend)[71] = (uint8_t)((crcFrame >> 8) & 0xFF);
    (*toSend)[72] = (uint8_t)(crcFrame & 0xFF);
  }
}
