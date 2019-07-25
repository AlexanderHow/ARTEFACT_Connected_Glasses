/*
  SSIManager.h - Library for computing and checking crc16 + generating ssi commands.
  Created by Pierre LeCoz and Alexandre Howard.
  7/25/2019
*/
#include <cstdlib>
#include <cstddef>
#include <stdint.h>
#include "config.h"

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
};

///////////////////
// Calcul du CRC //
///////////////////
uint16_t ssi_fnCRC16(const uint8_t *ptrFrame, uint16_t len);

///////////////////////////
// Vérification du CRC   //
///////////////////////////
static int ssi_frame_check_crc(const char *frame, uint16_t len);


///////////////////
//SEND SSI FRAME///
///////////////////
//reponse constante 0xFE 0x0F 0xF0 0x00 'a' 0x0100 FF00 0000 0000 0x5170 = 15 bytes
void SSI_queryRsp(uint8_t ** toSend, uint8_t lenFrame);

// 0x FE 2D D2 00 'n' 0050 "_Tera_evo_64_px_" "depth_1B" 01 00 000000 0000FF crc16 = 43 bytes
void SSI_discoverReply(uint8_t ** toSend, uint8_t lenFrame);

//0x FE 11 EE 00 'x' 0050 12 'L' 0008 12 'C' 0008 crc16 = 17 bytes
void SSI_configurationRsp(uint8_t ** toSend, uint8_t lenFrame);

//0x FE 08 F7 00 'y' 00 crc16 = 8 bytes 
void SSI_observerCreated(uint8_t ** toSend, uint8_t lenFrame);

//0x FE 49 B6 00 'm' 0050 64_data_of_1_byte crc16 = 73 bytes
void SSI_manyData(uint8_t ** toSend, uint8_t lenFrame, uint8_t * depths, int lenDepthArray);
