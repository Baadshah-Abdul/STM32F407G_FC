/*
 * hw-611.c
 *
 *  Created on: Jan 19, 2026
 *      Author: ACER1
 */
#include "hw-611.h"

void SPI2_GPIO_Config(void)		//setup PB13-PB15 for spi2
{
	GPIO_Handle_t SPI2pins;
	SPI2pins.pGPIOx = GPIOB;
	SPI2pins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPI2pins.GPIO_PinConfig.GPIO_PinAltFuncMode = 5;
	SPI2pins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPI2pins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPI2pins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	SPI2pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13; // SCK
	GPIO_Init(&SPI2pins);
	SPI2pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14; // MISO
	GPIO_Init(&SPI2pins);
	SPI2pins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15; // MOSI
	GPIO_Init(&SPI2pins);
}
//SPI2_SCK PB13
//SPI2_MISO	PB134
//SPI2_MOSI PB15

void SPI2_Config(void)				//spi2 setup
{
	SPI_Handle_t Spi_handle;
	Spi_handle.pSPIx = SPI2;
	SPI_PeriClockControl(SPI2, ENABLE);
	Spi_handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	Spi_handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;

	// Speed set to DIV8 (~2MHz) for cleaner data latching compared to DIV256
	Spi_handle.SPIConfig.SPI_SclkSpeed = SPI_SCLK_SPEED_DIV8;
	Spi_handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	Spi_handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	Spi_handle.SPIConfig.SPI_SSM = SPI_SSM_EN;
	SPI_Init(&Spi_handle);
	SPI_SSIConfig(SPI2, ENABLE);
}
void HW611_Config(void)			//CBS mode 00 for spi PB12
{
	GPIO_Handle_t HW_CS_Pin;
	GPIO_PeriClockControl(GPIOB, ENABLE);
	HW_CS_Pin.pGPIOx = GPIOB;
	HW_CS_Pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	HW_CS_Pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	HW_CS_Pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	HW_CS_Pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(&HW_CS_Pin);
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_12, GPIO_PIN_SET);
}

