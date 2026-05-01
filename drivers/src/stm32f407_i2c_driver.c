/*
 * stm32f407_i2c_driver.c
 *
 *  Created on: Feb 4, 2026
 *      Author: ACER1
 */

#include "stm32f407_i2c_driver.h"
#include "stm32f407_timer.h"


static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx);
static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr);
static void I2C_ClearAddrFlag(I2C_Handle_t *pI2CHandle);

static void I2C_GenerateStartCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_START);
}

static void I2C_ExecuteAddressPhaseRead(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr |= 1; //SlaveAddr is Slave address + r/nw bit = 1
	pI2Cx->I2C_DR = SlaveAddr;
}

static void I2C_ExecuteAddressPhaseWrite(I2C_RegDef_t *pI2Cx, uint8_t SlaveAddr)
{
	SlaveAddr = SlaveAddr << 1;
	SlaveAddr &= ~(1);//SlaveAddr is slaveaddress + r/nw bit = 0
	pI2Cx->I2C_DR = SlaveAddr;
}

static void I2C_ClearAddrFlag(I2C_Handle_t *pI2CHandle)
{
	uint32_t dummyRead;
	dummyRead = pI2CHandle->pI2Cx->I2C_SR1;
	dummyRead = pI2CHandle->pI2Cx->I2C_SR2;
	(void)dummyRead;

}

void I2C_GenerateStopCondition(I2C_RegDef_t *pI2Cx)
{
	pI2Cx->I2C_CR1 |= (1 << I2C_CR1_STOP);
}

void I2C_ManageAcking(I2C_RegDef_t *pI2Cx, uint8_t EnorDi)
{
	if(EnorDi == I2C_ACK_ENABLE)
	{
		//enable the ack
		pI2Cx->I2C_CR1 |= ( 1 << I2C_CR1_ACK);
	}else
	{
		//disable the ack
		pI2Cx->I2C_CR1 &= ~( 1 << I2C_CR1_ACK);
	}
}


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

uint8_t I2C_GetFlagStatus(I2C_RegDef_t *pI2Cx, uint32_t FlagName)
{
	if (pI2Cx->I2C_SR1 & FlagName)
	{
		return FLAG_SET;
	}
	return FLAG_RESET;
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle,uint8_t *pTxbuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	//generate START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//confirm start generation is completed by checking the SB flag in SR1
	//until SB is cleared SCL will be stretched(pulled to LOW)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

	//send the address of the slave with r/w bit set to w(0) (8 bits)
	I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, SlaveAddr);

	//wait until address is completed by checking ADDR flag in SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));

	//clear ADDR flag according to software sequence
	//until ADDR is cleared SCl will be stretched
	I2C_ClearAddrFlag(pI2CHandle);

	//send data when len becomes 0
	while(Len > 0)
	{
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));//Wait till TXE is set
		pI2CHandle->pI2Cx->I2C_DR = *pTxbuffer;
		pTxbuffer++;
		Len--;
	}

	//when len becomes 0 wait for TXE=1 and BTF=1 before generating STOP condition
	//Note: TXE=1 , BTF=1 , means that both SR and DR are empty and next transmission should begin
	//when BTF=1 SCL will be stretched (pulled to LOW)

	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));

	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF));


	//Generate STOP condition and master need not to wait for the completion of stop condition.
	//Note: generating STOP, automatically clears the BTF
	if(Sr == I2C_DISABLE_SR)
	{
		I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
	}
}


void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{
	//generate START condition
	I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

	//confirm start generation is completed by checking the SB flag in SR1
	//until SB is cleared SCL will be stretched(pulled to LOW)
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB));

	//send the address of the slave with r/w bit set to r(1) (8 bits)
	I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, SlaveAddr);

	//wait until address is completed by checking ADDR flag in SR1
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx,I2C_FLAG_ADDR));

	if(Len == 1)
	{
		//Disable Acking
		I2C_ManageAcking(pI2CHandle->pI2Cx,I2C_ACK_DISABLE);

		//clear ADDR flag
		I2C_ClearAddrFlag(pI2CHandle);

		//wait until RXNE is 1
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

		//generate STOP condition
		if(Sr == I2C_DISABLE_SR)
		{
			I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
		}
		//read data into buffer
		*pRxBuffer = pI2CHandle->pI2Cx->I2C_DR;
	}

    //procedure to read data from slave when Len > 1
	if(Len > 1)
	{
		//clear the ADDR flag
		I2C_ClearAddrFlag(pI2CHandle);

		for(uint32_t i = Len; i > 0; i--)
		{
			while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));

			//if last 2 bytes are remaining
			if(i == 2)
			{
				//disable ACK
				I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_DISABLE);
				//generate STOP condition
				if(Sr == I2C_DISABLE_SR)
				{
					I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
				}
			}
			//read data from register
			*pRxBuffer = pI2CHandle->pI2Cx->I2C_DR;
			//increment buffer address
			pRxBuffer++;
		}
	}
	//re-enable ACK
	if(pI2CHandle->I2CConfig.I2C_AckControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2CHandle->pI2Cx, I2C_ACK_ENABLE);
	}
}

