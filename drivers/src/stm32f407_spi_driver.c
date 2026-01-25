/*
 * stm32f407xx_spi_driver.c
 *
 * Created on: Sep 6, 2025
 * Author: ACER1
 */

#include "stm32f407_spi_driver.h"

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_rxe_interrupt_handle(SPI_Handle_t *pSPIHandle);
static void spi_ovr_interrupt_handle(SPI_Handle_t *pSPIHandle);

/* fn-     SPI_PeriClockControl
 *
 * brief-  Enables or disables the peripheral clock for the given SPI port
 *
 * param-  pSPIx: Base address of the SPI peripheral (SPI1, SPI2, or SPI3)
 * param-  EnOrDi: ENABLE or DISABLE macro
 *
 * return- None
 */
void SPI_PeriClockControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pSPIx == SPI1)
		{
			SPI1_PCLK_EN();
		}
		else if (pSPIx == SPI2)
		{
			SPI2_PCLK_EN();
		}
		else if (pSPIx == SPI3)
		{
			SPI3_PCLK_EN();
		}
	}
	else
	{
		if (pSPIx == SPI1)
		{
			SPI1_PCLK_DI();
		}
		else if (pSPIx == SPI2)
		{
			SPI2_PCLK_DI();
		}
		else if (pSPIx == SPI3)
		{
			SPI3_PCLK_DI();
		}
	}
}

/* fn-     SPI_Init
 *
 * brief-  Initializes the SPI peripheral according to the configuration settings
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure containing config and base address
 *
 * return- None
 */
void SPI_Init(SPI_Handle_t *pSPIHandle)
{
	uint32_t tempreg = 0;

	// Configure device mode
	tempreg |= pSPIHandle->SPIConfig.SPI_DeviceMode << SPI_CR1_MSTR;

	// Configure bus config
	if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_FD)
	{
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
	}
	else if (pSPIHandle->SPIConfig.SPI_BusConfig == SPI_BUS_CONFIG_HD)
	{
		tempreg |= (1 << SPI_CR1_BIDIMODE);
	}
	else if (pSPIHandle->SPIConfig.SPI_BusConfig
			== SPI_BUS_CONFIG_SIMPLEX_RXONLY)
	{
		tempreg &= ~(1 << SPI_CR1_BIDIMODE);
		tempreg |= (1 << SPI_CR1_RXONLY);
	}

	// Configure clock speed
	tempreg |= pSPIHandle->SPIConfig.SPI_SclkSpeed << SPI_CR1_BR;

	// Configure DFF
	tempreg |= pSPIHandle->SPIConfig.SPI_DFF << SPI_CR1_DFF;

	// Configure CPOL & CPHA
	tempreg |= pSPIHandle->SPIConfig.SPI_CPOL << SPI_CR1_CPOL;
	tempreg |= pSPIHandle->SPIConfig.SPI_CPHA << SPI_CR1_CPHA;

	// Configure SSM
	tempreg |= pSPIHandle->SPIConfig.SPI_SSM << SPI_CR1_SSM;

	// Write to CR1
	pSPIHandle->pSPIx->CR1 = tempreg;
}

/* fn-     SPI_DeInit
 *
 * brief-  Resets the registers of the given SPI peripheral
 *
 * param-  pSPIx: Base address of the SPI peripheral to reset
 *
 * return- None
 */
void SPI_DeInit(SPI_RegDef_t *pSPIx)
{
	if (pSPIx == SPI1)
	{
		SPI1_REG_RESET();
	}
	else if (pSPIx == SPI2)
	{
		SPI2_REG_RESET();
	}
	else if (pSPIx == SPI3)
	{
		SPI3_REG_RESET();
	}
}

/* fn-     SPI_GetFlagStatus
 *
 * brief-  Checks the status of a specific SPI flag in the Status Register (SR)
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  FlagName: The flag to check (e.g., SPI_TXE_FLAG, SPI_RXNE_FLAG)
 *
 * return- FLAG_SET if flag is up, FLAG_RESET otherwise
 */
uint8_t SPI_GetFlagStatus(SPI_RegDef_t *pSPIx, uint32_t FlagName)
{
	if (pSPIx->SR & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

/* fn-     SPI_SendData
 *
 * brief-  Transmits data over SPI (Blocking/Polling method)
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  pTxBuffer: Pointer to the data to be transmitted
 * param-  Len: Length of data in bytes
 *
 * return- None
 */
void SPI_SendData(SPI_RegDef_t *pSPIx, uint8_t *pTxBuffer, uint32_t Len)
{
	while (Len > 0)
	{
		while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET)
			;

		if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
		{
			pSPIx->DR = *((uint16_t*) pTxBuffer);
			Len -= 2;
			pTxBuffer += 2;
		}
		else
		{
			pSPIx->DR = *pTxBuffer;
			Len--;
			pTxBuffer++;
		}
	}
}

/* fn-     SPI_ReceiveData
 *
 * brief-  Receives data over SPI (Blocking/Polling method)
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  pRxBuffer: Pointer to the buffer where received data will be stored
 * param-  Len: Length of data in bytes
 *
 * return- None
 */
void SPI_ReceiveData(SPI_RegDef_t *pSPIx, uint8_t *pRxBuffer, uint32_t Len)
{
	while (Len > 0)
	{
		while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET)
			;

		if (pSPIx->CR1 & (1 << SPI_CR1_DFF))
		{
			*((uint16_t*) pRxBuffer) = (uint16_t) pSPIx->DR;
			Len -= 2;
			pRxBuffer += 2;
		}
		else
		{
			*pRxBuffer = (uint8_t) pSPIx->DR;
			Len--;
			pRxBuffer++;
		}
	}
}

/* fn-     SPI_TransmitReceive
 *
 * brief-  Sends and receives a single byte simultaneously (Standard SPI transaction)
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  data: The byte to transmit
 *
 * return- The byte received during the transaction
 */
uint8_t SPI_TransmitReceive(SPI_RegDef_t *pSPIx, uint8_t data)
{
	while (SPI_GetFlagStatus(pSPIx, SPI_TXE_FLAG) == FLAG_RESET)
		;
	pSPIx->DR = data;
	while (SPI_GetFlagStatus(pSPIx, SPI_RXNE_FLAG) == FLAG_RESET)
		;
	return (uint8_t) pSPIx->DR;
}

/* fn-     SPI_PeripheralControl
 *
 * brief-  Enables or disables the SPI peripheral (controls the SPE bit)
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  EnOrDi: ENABLE or DISABLE macro
 *
 * return- None
 */
void SPI_PeripheralControl(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pSPIx->CR1 |= (1 << SPI_CR1_SPE);
	}
	else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SPE);
	}
}

/* fn-     SPI_SSIConfig
 *
 * brief-  Configures the Internal Slave Select (SSI) bit
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  EnOrDi: ENABLE (Sets SSI to 1) or DISABLE (Sets SSI to 0)
 *
 * return- None
 */
void SPI_SSIConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pSPIx->CR1 |= (1 << SPI_CR1_SSI);
	}
	else
	{
		pSPIx->CR1 &= ~(1 << SPI_CR1_SSI);
	}
}

/* fn-     SPI_SSOEConfig
 *
 * brief-  Configures the Slave Select Output Enable (SSOE) bit
 *
 * param-  pSPIx: Base address of the SPI peripheral
 * param-  EnOrDi: ENABLE or DISABLE macro
 *
 * return- None
 */
void SPI_SSOEConfig(SPI_RegDef_t *pSPIx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pSPIx->CR2 |= (1 << SPI_CR2_SSOE);
	}
	else
	{
		pSPIx->CR2 &= ~(1 << SPI_CR2_SSOE);
	}
}

/* fn-     SPI_IRQInterruptConfig
 *
 * brief-  Enables or disables a specific SPI interrupt in the NVIC
 *
 * param-  IRQNumber: The IRQ number to configure
 * param-  EnorDi: ENABLE or DISABLE macro
 *
 * return- None
 */
void SPI_IRQInterruptConfig(uint8_t IRQNumber, uint8_t EnorDi)
{
	if (EnorDi == ENABLE)
	{
		if (IRQNumber <= 31)
		{
			*NVIC_ISER0 |= (1 << IRQNumber);
		}
		else if (IRQNumber < 64)
		{
			*NVIC_ISER1 |= (1 << (IRQNumber % 32));
		}
		else if (IRQNumber < 96)
		{
			*NVIC_ISER2 |= (1 << (IRQNumber % 64));
		}
	}
	else
	{
		if (IRQNumber <= 31)
		{
			*NVIC_ICER0 |= (1 << IRQNumber);
		}
		else if (IRQNumber < 64)
		{
			*NVIC_ICER1 |= (1 << (IRQNumber % 32));
		}
		else if (IRQNumber < 96)
		{
			*NVIC_ICER2 |= (1 << (IRQNumber % 64));
		}
	}
}

/* fn-     SPI_IRQPriorityConfig
 *
 * brief-  Configures the priority level of an SPI interrupt
 *
 * param-  IRQNumber: The IRQ number
 * param-  IRQPriority: Priority value (0-15)
 *
 * return- None
 */
void SPI_IRQPriorityConfig(uint8_t IRQNumber, uint32_t IRQPriority)
{
	uint8_t iprx = IRQNumber / 4;
	uint8_t iprx_section = IRQNumber % 4;
	uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
	*(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);
}

/* fn-     SPI_SendDataIT
 *
 * brief-  Initiates SPI data transmission using an Interrupt-driven (Non-blocking) method
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure
 * param-  pTxBuffer: Pointer to the data buffer
 * param-  Len: Number of bytes to send
 *
 * return- Current state of the transmission (SPI_BUSY_IN_TX or SPI_READY)
 */
uint8_t SPI_SendDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pTxBuffer,
		uint32_t Len)
{
	uint8_t state = pSPIHandle->TxState;
	if (state != SPI_BUSY_IN_TX)
	{
		pSPIHandle->pTxBuffer = pTxBuffer;
		pSPIHandle->TxLen = Len;
		pSPIHandle->TxState = SPI_BUSY_IN_TX;
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_TXEIE);
	}
	return state;
}

/* fn-     SPI_ReceiveDataIT
 *
 * brief-  Initiates SPI data reception using an Interrupt-driven (Non-blocking) method
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure
 * param-  pRxBuffer: Pointer to the buffer to store data
 * param-  Len: Number of bytes to receive
 *
 * return- Current state of the reception (SPI_BUSY_IN_RX or SPI_READY)
 */
uint8_t SPI_ReceiveDataIT(SPI_Handle_t *pSPIHandle, uint8_t *pRxBuffer,
		uint32_t Len)
{
	uint8_t state = pSPIHandle->RxState;
	if (state != SPI_BUSY_IN_RX)
	{
		pSPIHandle->pRxBuffer = pRxBuffer;
		pSPIHandle->RxLen = Len;
		pSPIHandle->RxState = SPI_BUSY_IN_RX;
		pSPIHandle->pSPIx->CR2 |= (1 << SPI_CR2_RXNEIE);
	}
	return state;
}

static void spi_txe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
	{
		pSPIHandle->pSPIx->DR = *((uint16_t*) pSPIHandle->pTxBuffer);
		pSPIHandle->TxLen -= 2;
		pSPIHandle->pTxBuffer += 2;
	}
	else
	{
		pSPIHandle->pSPIx->DR = *pSPIHandle->pTxBuffer;
		pSPIHandle->TxLen--;
		pSPIHandle->pTxBuffer++;
	}

	if (!pSPIHandle->TxLen)
	{
		SPI_CloseTransmisson(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_TX_CMPLT);
	}
}
static void spi_rxe_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	if (pSPIHandle->pSPIx->CR1 & (1 << SPI_CR1_DFF))
	{
		*((uint16_t*) pSPIHandle->pRxBuffer) = (uint16_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen -= 2;
		pSPIHandle->pRxBuffer += 2;
	}
	else
	{
		*pSPIHandle->pRxBuffer = (uint8_t) pSPIHandle->pSPIx->DR;
		pSPIHandle->RxLen--;
		pSPIHandle->pRxBuffer++;
	}

	if (!pSPIHandle->RxLen)
	{
		SPI_CloseReception(pSPIHandle);
		SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_RX_CMPLT);
	}
}
static void spi_ovr_interrupt_handle(SPI_Handle_t *pSPIHandle)
{
	if (pSPIHandle->TxState != SPI_BUSY_IN_TX)
	{
		volatile uint8_t temp;
		temp = pSPIHandle->pSPIx->DR;
		temp = pSPIHandle->pSPIx->SR;
		(void) temp;
	}
	SPI_ApplicationEventCallback(pSPIHandle, SPI_EVENT_OVR_ERR);
}

/* fn-     SPI_IRQHandling
 *
 * brief-  Master interrupt handler for SPI events (TXE, RXNE, and Errors)
 *
 * param-  pHandle: Pointer to the SPI handle structure
 *
 * return- None
 */
void SPI_IRQHandling(SPI_Handle_t *pSPIHandle)
{
	uint8_t temp1, temp2;

	// TXE handling
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_TXE);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_TXEIE);
	if (temp1 && temp2)
	{
		spi_txe_interrupt_handle(pSPIHandle);
	}

	// RXNE handling
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_RXNE);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_RXNEIE);
	if (temp1 && temp2)
	{
		spi_rxe_interrupt_handle(pSPIHandle);
	}

	// OVR(overrun) error handling
	temp1 = pSPIHandle->pSPIx->SR & (1 << SPI_SR_OVR);
	temp2 = pSPIHandle->pSPIx->CR2 & (1 << SPI_CR2_ERRIE);
	if (temp1 && temp2)
	{
		spi_ovr_interrupt_handle(pSPIHandle);
	}
}

/* fn-     SPI_CloseTransmisson
 *
 * brief-  Closes an active SPI transmission and resets the handle state
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure
 *
 * return- None
 */
void SPI_CloseTransmisson(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_TXEIE);
	pSPIHandle->pTxBuffer = NULL;
	pSPIHandle->TxLen = 0;
	pSPIHandle->TxState = SPI_READY;
}

/* fn-     SPI_CloseReception
 *
 * brief-  Closes an active SPI reception and resets the handle state
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure
 *
 * return- None
 */
void SPI_CloseReception(SPI_Handle_t *pSPIHandle)
{
	pSPIHandle->pSPIx->CR2 &= ~(1 << SPI_CR2_RXNEIE);
	pSPIHandle->pRxBuffer = NULL;
	pSPIHandle->RxLen = 0;
	pSPIHandle->RxState = SPI_READY;
}

/* fn-     SPI_ClearOVRFlag
 *
 * brief-  Clears the Overrun (OVR) flag by performing a sequence of reads
 *
 * param-  pSPIx: Base address of the SPI peripheral
 *
 * return- None
 */
void SPI_ClearOVRFlag(SPI_RegDef_t *pSPIx)
{
	volatile uint8_t temp;
	temp = pSPIx->DR;
	temp = pSPIx->SR;
	(void) temp;
}

/* fn-     SPI_ApplicationEventCallback
 *
 * brief-  Weak implementation of the application event callback
 *
 * param-  pSPIHandle: Pointer to the SPI handle structure
 * param-  AppEv: The event that occurred (e.g., SPI_EVENT_TX_CMPLT)
 *
 * return- None
 */
__weak void SPI_ApplicationEventCallback(SPI_Handle_t *pSPIHandle,
		uint8_t AppEv)
{
	// User can override this in application
}
