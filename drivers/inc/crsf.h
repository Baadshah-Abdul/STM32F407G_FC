/*
 * crsf.h
 *
 *  Created on: May 3, 2026
 *      Author: Abdul
 */

#ifndef INC_CRSF_H_
#define INC_CRSF_H_

#include "stm32f407.h"
#include "stm32f407_usart_driver.h"

// CRSF protocol constants
#define CRSF_ADDRESS         0xC8
#define CRSF_TYPE_RC_CHANNELS 0x16
#define CRSF_PAYLOAD_LEN     22
#define CRSF_PACKET_LEN      26  // addr + len + type + 22 payload + crc

// Channel count and range
#define CRSF_NUM_CHANNELS    16
#define CRSF_CH_MIN          172
#define CRSF_CH_MID          992
#define CRSF_CH_MAX          1811

// Parser states
#define CRSF_STATE_WAIT_ADDR    0
#define CRSF_STATE_WAIT_LEN     1
#define CRSF_STATE_WAIT_TYPE    2
#define CRSF_STATE_READ_PAYLOAD 3
#define CRSF_STATE_READ_CRC     4

typedef struct
{
    uint16_t ch[CRSF_NUM_CHANNELS];  // raw 11-bit values
    uint8_t  valid;                   // 1 = fresh valid packet
} CRSF_Data_t;

void     CRSF_Init(void);
void     CRSF_ParseByte(uint8_t byte);
CRSF_Data_t* CRSF_GetChannels(void);
uint16_t CRSF_ToUs(uint16_t raw);     // converts raw to µs



#endif /* INC_CRSF_H_ */
