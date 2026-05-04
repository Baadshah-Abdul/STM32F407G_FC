/*
 * crsf.c
 *
 *  Created on: May 3, 2026
 *      Author: Abdul
 */

#include "crsf.h"
#include "stm32f407_gpio_driver.h"

static CRSF_Data_t crsf_data;
static uint8_t state = CRSF_STATE_WAIT_ADDR;
static uint8_t payload[CRSF_PAYLOAD_LEN];
static uint8_t payload_idx = 0;
static uint8_t packet_len = 0;
static uint8_t packet_type = 0;

static uint8_t CRSF_CRC8(uint8_t *data, uint8_t len);
static void CRSF_UnpackChannels(void);

// CRC8 DVB-S2
static uint8_t CRSF_CRC8(uint8_t *data, uint8_t len)
{
	// initialise crc accumulator to 0
	uint8_t crc_acc = 0;
	// loop over each byte in data for len bytes
	for (uint32_t i = 0; i < len; i++)
	{
		// XOR current byte into crc
		crc_acc ^= *data;
		data++;
		for (uint8_t j = 0; j < 8; j++)
		{
			// process each of the 8 bits
			// if the top bit (bit 7) of crc is set
			if (crc_acc & 0x80)
			{
				// shift crc left by 1
				crc_acc = (crc_acc << 1);
				// XOR crc with polynomial 0xD5
				crc_acc ^= 0xD5;
			}
			else
			{   // else
				// shift crc left by 1
				crc_acc = (crc_acc << 1);
			}
		}
	}
	// return crc
	return crc_acc;
}

static void CRSF_UnpackChannels(void)
{
	// CRSF packs 16 channels as 11-bit values into 22 bytes
	// Each channel value needs to be masked with 0x07FF
	// to keep only the lower 11 bits

	// channel 0 — bits start at bit 0 of payload[0]
	crsf_data.ch[0] = ((uint32_t) payload[0] | (uint32_t) payload[1] << 8)
			& 0x07FF;
	// payload[0] | payload[1] shifted left 8
	// mask with 0x07FF

	// channel 1 — bits start at bit 3 of payload[1]
	crsf_data.ch[1] = (((uint32_t) payload[1] >> 3)
			| ((uint32_t) payload[2] << 5)) & 0x07FF;
	// payload[1] shifted right 3
	// OR payload[2] shifted left 5
	// mask with 0x07FF

	// channel 2 — bits start at bit 6 of payload[2]
	crsf_data.ch[2] = (((uint32_t) payload[2] >> 6)
			| ((uint32_t) payload[3] << 2) | ((uint32_t) payload[4] << 10))
			& 0x07FF;
	// payload[2] shifted right 6
	// OR payload[3] shifted left 2
	// OR payload[4] shifted left 10
	// mask with 0x07FF

	// channel 3 — bits start at bit 1 of payload[4]
	crsf_data.ch[3] = (((uint32_t) payload[4] >> 1)
			| ((uint32_t) payload[5] << 7)) & 0x07FF;
	// payload[4] shifted right 1
	// OR payload[5] shifted left 7
	// mask with 0x07FF

	// channel 4 — bits start at bit 4 of payload[5]
	crsf_data.ch[4] = ((uint32_t) payload[5] >> 4 | (uint32_t) payload[6] << 4)
			& 0x07FF;
	// payload[5] shifted right 4
	// OR payload[6] shifted left 4
	// mask with 0x07FF

	// channel 5 — bits start at bit 7 of payload[6]
	crsf_data.ch[5] = ((uint32_t) payload[6] >> 7 | (uint32_t) payload[7] << 1
			| (uint32_t) payload[8] << 9) & 0x07FF;
	// payload[6] shifted right 7
	// OR payload[7] shifted left 1
	// OR payload[8] shifted left 9
	// mask with 0x07FF

	// channel 6 — bits start at bit 2 of payload[8]
	crsf_data.ch[6] = ((uint32_t) payload[8] >> 2 | (uint32_t) payload[9] << 6)
			& 0x07FF;
	// payload[8] shifted right 2
	// OR payload[9] shifted left 6
	// mask with 0x07FF

	// channel 7 — bits start at bit 5 of payload[9]
	crsf_data.ch[7] = ((uint32_t) payload[9] >> 5 | (uint32_t) payload[10] << 3)
			& 0x07FF;
	// payload[9] shifted right 5
	// OR payload[10] shifted left 3
	// mask with 0x07FF

	// channel 8 — bits start at bit 0 of payload[11]
	crsf_data.ch[8] = ((uint32_t) payload[11] | (uint32_t) payload[12] << 8)
			& 0x07FF;
	// payload[11] | payload[12] shifted left 8
	// mask with 0x07FF

	// channel 9 — bits start at bit 3 of payload[12]
	crsf_data.ch[9] =
			((uint32_t) payload[12] >> 3 | (uint32_t) payload[13] << 5)
					& 0x07FF;
	// payload[12] shifted right 3
	// OR payload[13] shifted left 5
	// mask with 0x07FF

	// channel 10 — bits start at bit 6 of payload[13]
	crsf_data.ch[10] = (((uint32_t) payload[13] >> 6)
			| (uint32_t) payload[14] << 2 | (uint32_t) payload[15] << 10)
			& 0x07FF;
	// payload[13] shifted right 6
	// OR payload[14] shifted left 2
	// OR payload[15] shifted left 10
	// mask with 0x07FF

	// channel 11 — bits start at bit 1 of payload[15]
	crsf_data.ch[11] = ((uint32_t) payload[15] >> 1
			| (uint32_t) payload[16] << 7) & 0x07FF;
	// payload[15] shifted right 1
	// OR payload[16] shifted left 7
	// mask with 0x07FF

	// channel 12 — bits start at bit 4 of payload[16]
	crsf_data.ch[12] = ((uint32_t) payload[16] >> 4
			| (uint32_t) payload[17] << 4) & 0x07FF;
	// payload[16] shifted right 4
	// OR payload[17] shifted left 4
	// mask with 0x07FF

	// channel 13 — bits start at bit 7 of payload[17]
	crsf_data.ch[13] = ((uint32_t) payload[17] >> 7
			| (uint32_t) payload[18] << 1 | ((uint32_t) payload[19] << 9))
			& 0x07FF;
	// payload[17] shifted right 7
	// OR payload[18] shifted left 1
	// OR payload[19] shifted left 9
	// mask with 0x07FF

	// channel 14 — bits start at bit 2 of payload[19]
	crsf_data.ch[14] = ((uint32_t) payload[19] >> 2
			| (uint32_t) payload[20] << 6) & 0x07FF;
	// payload[19] shifted right 2
	// OR payload[20] shifted left 6
	// mask with 0x07FF

	// channel 15 — bits start at bit 5 of payload[20]
	crsf_data.ch[15] = ((uint32_t) payload[20] >> 5
			| (uint32_t) payload[21] << 3) & 0x07FF;
	// payload[20] shifted right 5
	// OR payload[21] shifted left 3
	// mask with 0x07FF
}

// Called from USART2_IRQHandler one byte at a time
void CRSF_ParseByte(uint8_t byte)
{
	switch (state)
	{
	case CRSF_STATE_WAIT_ADDR:
		// if byte equals CRSF_ADDRESS
		if (byte == CRSF_ADDRESS)
		{
			// set state to CRSF_STATE_WAIT_LEN
			state = CRSF_STATE_WAIT_LEN;
		}
		break;

	case CRSF_STATE_WAIT_LEN:
		// store byte into packet_len
		packet_len = byte;
		// set state to CRSF_STATE_WAIT_TYPE
		state = CRSF_STATE_WAIT_TYPE;
		// break
		break;

	case CRSF_STATE_WAIT_TYPE:
		// if byte equals CRSF_TYPE_RC_CHANNELS
		if (byte == CRSF_TYPE_RC_CHANNELS)
		{
			// store byte into packet_type
			packet_type = byte;
			// reset payload_idx to 0
			payload_idx = 0;
			// set state to CRSF_STATE_READ_PAYLOAD
			state = CRSF_STATE_READ_PAYLOAD;
		}
		else
		{
			// reset state to CRSF_STATE_WAIT_ADDR
			state = CRSF_STATE_WAIT_ADDR;
		}
		break;

	case CRSF_STATE_READ_PAYLOAD:
		// store byte into payload at payload_idx
		payload[payload_idx] = byte;
		// increment payload_idx
		payload_idx++;
		// if payload_idx equals CRSF_PAYLOAD_LEN
		if (payload_idx == CRSF_PAYLOAD_LEN)
		{
			// set state to CRSF_STATE_READ_CRC
			state = CRSF_STATE_READ_CRC;
		}
		break;

	case CRSF_STATE_READ_CRC:
	{
		// declare a local variable to hold computed crc
		uint8_t comp_crc;
		uint8_t crc_buf[1 + CRSF_PAYLOAD_LEN];

		// compute CRC8 over packet_type byte and payload
		// hint: CRC covers type + payload, type is 1 byte
		//       pass address of packet_type, length = 1 + CRSF_PAYLOAD_LEN
		crc_buf[0] = packet_type;

		for (uint8_t i = 0; i < CRSF_PAYLOAD_LEN; i++)
		{
			crc_buf[i + 1] = payload[i];
		}
		comp_crc = CRSF_CRC8(crc_buf, 1 + CRSF_PAYLOAD_LEN);

		// if computed crc equals received byte
		if (comp_crc == byte)
		{
			// unpack channels — call a helper CRSF_UnpackChannels()
			CRSF_UnpackChannels();
			// set crsf_data.valid to 1
			crsf_data.valid = 1;
		}
		else
		{
			// set crsf_data.valid to 0
			crsf_data.valid = 0;
		}
		// always reset state to CRSF_STATE_WAIT_ADDR
		state = CRSF_STATE_WAIT_ADDR;
		break;
	}

	default:
		// reset state to CRSF_STATE_WAIT_ADDR
		state = CRSF_STATE_WAIT_ADDR;
		break;

	}
}

// USART2 GPIO — PA3 as AF7
// USART2 init — 420000 baud, RX only, RXINV enabled
void CRSF_Init(void)
{
	// ── PA3 GPIO config ──────────────────────────────

	// declare a GPIO_Handle_t
	GPIO_Handle_t USART_Pin;
	// set port to GPIOA
	USART_Pin.pGPIOx = GPIOA;
	// set pin number to PA3
	USART_Pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	// set mode to alternate function
	USART_Pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	// set alternate function to AF7 (USART2_RX)
	USART_Pin.GPIO_PinConfig.GPIO_PinAltFuncMode = 7;
	// set output type to push-pull
	USART_Pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	// set pull-up/pull-down to no pull
	USART_Pin.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	// set speed to fast
	USART_Pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	// call GPIO_Init()
	GPIO_Init(&USART_Pin);

	// ── USART2 config ────────────────────────────────

	// declare a USART_Handle_t
	USART_Handle_t CRSF_Pins;
	// set peripheral to USART2
	CRSF_Pins.pUSARTx = USART2;
	// set mode to RX only
	CRSF_Pins.USART_Config.USART_Mode = USART_MODE_ONLY_RX;
	// set baud to 420000
	CRSF_Pins.USART_Config.USART_Baud = USART_CRSF_BAUD_420000;
	// set word length to 8 bits
	CRSF_Pins.USART_Config.USART_WordLength = USART_WORDLEN_8BITS;
	// set stop bits to 1
	CRSF_Pins.USART_Config.USART_NoOfStopBits = USART_STOPBITS_1;
	// set parity to disable
	CRSF_Pins.USART_Config.USART_ParityControl = USART_PARITY_DISABLE;
	// set hardware flow control to none
	CRSF_Pins.USART_Config.USART_HWFlowControl = USART_HW_FLOW_CTRL_NONE;
	// set RxInvert to enable  ← critical for CRSF
	CRSF_Pins.USART_Config.USART_RxInvert = USART_RX_INVERT_EN;
	// call USART_Init()
	USART_Init(&CRSF_Pins);

	// ── Enable interrupt ─────────────────────────────

	// call USART_IRQInterruptConfig with IRQ_NO_USART2, ENABLE
	USART_IRQInterruptConfig(IRQ_NO_USART2, ENABLE);
	// set priority for IRQ_NO_USART2 — use a mid level priority
	USART_IRQPriorityConfig(IRQ_NO_USART2, NVIC_IRQ_PRI7);
	// enable RXNE interrupt bit in USART2 CR1
	USART2->USART_CR1 |= (1 << USART_CR1_RXNEIE);
	// call USART_PeripheralControl to enable USART2
	USART_PeripheralControl(CRSF_Pins.pUSARTx, ENABLE);
}

// Returns pointer to channel data
CRSF_Data_t* CRSF_GetChannels(void)
{
	return &crsf_data;
}

// Maps raw 172-1811 to 1000-2000µs
uint16_t CRSF_ToUs(uint16_t raw)
{
    // map raw range 172-1811 to output range 1000-2000
    // formula: ((raw - CRSF_CH_MIN) * 1000) / (CRSF_CH_MAX - CRSF_CH_MIN) + 1000
    // cast to uint32_t before multiplying to prevent overflow
    // return result as uint16_t
	raw = (((uint32_t)(raw - CRSF_CH_MIN)) * 1000) / (CRSF_CH_MAX - CRSF_CH_MIN) + 1000;
	return (uint16_t)raw;
}

// USART2 IRQ handler — reads DR, calls ParseByte
void USART2_IRQHandler(void)
{
    // check if RXNE flag is set in USART2 SR
	if(USART2->USART_SR & USART_FLAG_RXNE)
	{
		uint8_t temp;
        // read one byte from USART2 DR into a uint8_t
		temp = USART2->USART_DR;
        // pass that byte to CRSF_ParseByte()
		CRSF_ParseByte(temp);
	}
}
