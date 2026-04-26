/*
 * stm32f407_i2c_driver.c
 *
 *  Created on: Feb 4, 2026
 *      Author: ACER1
 */

#include "stm32f407_i2c_driver.h"

void I2C_PeripheralControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		pI2Cx->I2C_CR1 |= I2C_CR1_PE;
	}
	else
	{
		pI2Cx->I2C_CR1 &= ~(1 << 0);
	}
}

void I2C_PeriClockControl(I2C_RegDef_t *pI2Cx, uint8_t EnOrDi)
{
	if (EnOrDi == ENABLE)
	{
		if (pI2Cx == I2C1)
		{
			I2C1_PCLK_EN();
		}
		else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_EN();
		}
		else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_EN();
		}
	}
	else
	{
		if (pI2Cx == I2C1)
		{
			I2C1_PCLK_DI();
		}
		else if (pI2Cx == I2C2)
		{
			I2C2_PCLK_DI();
		}
		else if (pI2Cx == I2C3)
		{
			I2C3_PCLK_DI();
		}
	}
}


void I2C_Init(I2C_Handle_t *pI2CHandle)
{
	uint32_t tempreg = 0;

	//enable clk for i2cx peripehral
	I2C_PeriClockControl(pI2CHandle->pI2Cx, ENABLE);

	//CR1 Configuration (ACK)
	tempreg = pI2CHandle->I2CConfig.I2C_AckControl  << I2C_CR1_ACK;
	pI2CHandle->pI2Cx->I2C_CR1 = tempreg;

	//CR2 Configuration (FREQ)
	tempreg = Get_PeriCLK() /1000000;
	//make 0-5 bit = 0
	pI2CHandle->pI2Cx->I2C_CR2 = (tempreg & 0x3F);

	//OAR1 Configuration (Address)
	tempreg = (pI2CHandle->I2CConfig.I2C_DeviceAddress << I2C_OAR1_ADD1);
	tempreg |= (1 << 14);
	pI2CHandle->pI2Cx->I2C_OAR1 = tempreg;

	//CCR Configuration (The "Heartbeat")
	tempreg = Get_PeriCLK();
	if (pI2CHandle->I2CConfig.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)	{
		// Standard Mode: PCLK / (2 * Speed)
		// Bit 15 is 0 by default
		tempreg = tempreg / (2 * pI2CHandle->I2CConfig.I2C_SCLSpeed);
	}
	else
	{
		// Fast Mode: PCLK / (3 * Speed)
		uint32_t ccr_math = (tempreg / (3 * pI2CHandle->I2CConfig.I2C_SCLSpeed));
		//Clear tempreg and build the register value from scratch
		tempreg = (1 << 15);//fast bit
		tempreg |= (pI2CHandle->I2CConfig.I2C_FMDutyCycle << 14);//duty bit
		//Add the math result
		tempreg |= (ccr_math & 0xFFF);
	}
	pI2CHandle->pI2Cx->I2C_CCR = tempreg;


	//TRISE Configuration
	tempreg = Get_PeriCLK() / 1000000;
	if(pI2CHandle->I2CConfig.I2C_SCLSpeed <= I2C_SCL_SPEED_SM)
	{
		tempreg += 1;
	}
	else
	{
		// Fast Mode (300ns max rise time)
		tempreg = ((tempreg * 300) / 1000) + 1;
	}
	pI2CHandle->pI2Cx->I2C_TRISE = tempreg;

	//Enable the Peripheral
	I2C_PeripheralControl(pI2CHandle->pI2Cx, ENABLE);

}


void I2C_DeInit(I2C_RegDef_t *pI2Cx)
{
    if (pI2Cx == I2C1)
    {
        I2C1_REG_RESET();   // your macro that pulses RCC reset bit
    }
    else if (pI2Cx == I2C2)
    {
        I2C2_REG_RESET();
    }
    else if (pI2Cx == I2C3)
    {
        I2C3_REG_RESET();
    }
}
