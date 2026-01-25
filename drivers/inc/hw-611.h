/*
 * hw-611.h
 *
 *  Created on: Jan 19, 2026
 *      Author: ACER1
 */

#ifndef INC_HW_611_H_
#define INC_HW_611_H_

#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_spi_driver.h"

#define WHOAMI 	0x58			//at addr 0xD0
#define RESET_VAL	0xB6		//at addr 0xE0
#define STATUS 		0xF3		//stores 	status of device
#define CTRL		0xF4		//sets data acquisition options of device
/*

 Bit 7, 6, 5 osrs_t[2:0] Controls oversampling of temperature data. See chapter
 3.3.2 for details.
 Bit 4, 3, 2 osrs_p[2:0] Controls oversampling of pressure data. See chapter
 3.3.1 for details.
 Bit 1, 0 mode[1:0] Controls the power mode of the device. See chapter 3.6 for details
 */
#define CONFIG		0xF5		//sets the rate, filter and interface options of the device

void SPI2_GPIO_Config(void);		//setup PB12-PB15 for spi2

//SPI2_NSS PB12
//SPI2_SCK PB13
//SPI2_MISO	PB134
//SPI2_MOSI PB15

void SPI2_Config(void);

void HW611_Config(void);	//CBS mode 00 for spi

//readfunc
//writefunc

//processdata

#endif /* INC_HW_611_H_ */
