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

static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle );
static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle );

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
	uint8_t dummy_read;
	//check device mode
	if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
	{
		//device Master mode
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize == 1)
			{
				//disable ACK
				I2C_ManageAcking(pI2CHandle->pI2Cx, DISABLE);
				//clear ADDR flag(read SR1,SR2)
				dummy_read = pI2CHandle->pI2Cx->I2C_SR1;
				dummy_read = pI2CHandle->pI2Cx->I2C_SR2;
				(void)dummy_read;
			}
		}else
		{
			dummy_read = pI2CHandle->pI2Cx->I2C_SR1;
			dummy_read = pI2CHandle->pI2Cx->I2C_SR2;
			(void)dummy_read;
		}
	}else
	{
		//device Slave mode
		dummy_read = pI2CHandle->pI2Cx->I2C_SR1;
		dummy_read = pI2CHandle->pI2Cx->I2C_SR2;
		(void)dummy_read;
	}

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

/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
uint8_t  I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len,uint8_t SlaveAddr, uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->SR = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITERREN);

	}

	return busystate;

}

/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer, uint8_t Len, uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = pI2CHandle->RxSize; //Rxsize is used in the ISR code to manage the data reception
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->SR = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateStartCondition(pI2CHandle->pI2Cx);

		//Implement the code to enable ITBUFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITBUFEN);

		//Implement the code to enable ITEVFEN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITEVTEN);

		//Implement the code to enable ITERREN Control Bit
		pI2CHandle->pI2Cx->I2C_CR2 |= (1 << I2C_CR2_ITERREN);
	}

	return busystate;
}


void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITEVTEN);

	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pRxBuffer = NULL;
	pI2CHandle->RxLen = 0;
	pI2CHandle->RxSize = 0;

	if(pI2CHandle->I2CConfig.I2C_AckControl == I2C_ACK_ENABLE)
	{
		I2C_ManageAcking(pI2CHandle->pI2Cx,ENABLE);
	}

}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
	//Implement the code to disable ITBUFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITBUFEN);

	//Implement the code to disable ITEVFEN Control Bit
	pI2CHandle->pI2Cx->I2C_CR2 &= ~( 1 << I2C_CR2_ITEVTEN);


	pI2CHandle->TxRxState = I2C_READY;
	pI2CHandle->pTxBuffer = NULL;
	pI2CHandle->TxLen = 0;
}

static void I2C_MasterHandleTXEInterrupt(I2C_Handle_t *pI2CHandle )
{

	if(pI2CHandle->TxLen > 0)
	{
		//1. load the data in to DR
		pI2CHandle->pI2Cx->I2C_DR = *(pI2CHandle->pTxBuffer);

		//2. decrement the TxLen
		pI2CHandle->TxLen--;

		//3. Increment the buffer address
		pI2CHandle->pTxBuffer++;

	}

}




static void I2C_MasterHandleRXNEInterrupt(I2C_Handle_t *pI2CHandle )
{
	//We have to do the data reception
	if(pI2CHandle->RxSize == 1)
	{
		*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->I2C_DR;
		pI2CHandle->RxLen--;
	}


	if(pI2CHandle->RxSize > 1)
	{
		if(pI2CHandle->RxLen == 2)
		{
			//clear the ack bit
			I2C_ManageAcking(pI2CHandle->pI2Cx,DISABLE);
		}

			//read DR
			*pI2CHandle->pRxBuffer = pI2CHandle->pI2Cx->I2C_DR;
			pI2CHandle->pRxBuffer++;
			pI2CHandle->RxLen--;
	}

	if(pI2CHandle->RxLen == 0 )
	{
		//close the I2C data reception and notify the application

		//1. generate the stop condition
		if(pI2CHandle->SR == I2C_DISABLE_SR)
			I2C_GenerateStopCondition(pI2CHandle->pI2Cx);

		//2 . Close the I2C rx
		I2C_CloseReceiveData(pI2CHandle);

		//3. Notify the application
		I2C_ApplicationEventCallback(pI2CHandle,I2C_EV_RX_CMPLT);
	}
}








void I2C_SlaveSendData(I2C_RegDef_t *pI2C,uint8_t data)
{
	pI2C->I2C_DR = data;
}
uint8_t I2C_SlaveReceiveData(I2C_RegDef_t *pI2C)
{
	return (uint8_t)pI2C->I2C_DR;
}


/*********************************************************************
 * @fn      		  - I2C_EV_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Interrupt handling for different I2C events (refer SR1)

 */
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{

	//Interrupt handling for both master and slave mode of a device
	uint8_t temp1, temp2, temp3;

	temp1 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITEVTEN);
	temp2 = pI2CHandle->pI2Cx->I2C_CR2 & (1 << I2C_CR2_ITBUFEN);

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_SB);

	//1. Handle For interrupt generated by SB event
	//	Note : SB flag is only applicable in Master mode
	if(temp1 && temp3)
	{
		//SB flag id set
		//will not be executed in slave mode
		//execute address phase
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			I2C_ExecuteAddressPhaseWrite(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}
		else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_ExecuteAddressPhaseRead(pI2CHandle->pI2Cx, pI2CHandle->DevAddr);
		}

	}


	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_ADDR);
	//2. Handle For interrupt generated by ADDR event
	//Note : When master mode : Address is sent
	//		 When Slave mode   : Address matched with own address
	if(temp1 && temp3)
	{
		//ADDR flag id set
		I2C_ClearAddrFlag(pI2CHandle);
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_BTF);
	//3. Handle For interrupt generated by BTF(Byte Transfer Finished) event
	if(temp1 && temp3)
	{
		//BTF flag id set
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			//make sure TXE set
			if(pI2CHandle->TxRxState == I2C_SR1_TXE)
			{
				//BTF, TXE = 1
				if(pI2CHandle->TxLen == 0)
				{
					//generate stop
					if(pI2CHandle->SR == I2C_DISABLE_SR)
					{
						I2C_GenerateStopCondition(pI2CHandle->pI2Cx);
					}
					//reset all member elements of handle struct
					I2C_CloseReceiveData(pI2CHandle);
					//notify appication abt transmission complete
					I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_TX_CMPLT);
				}
			}
		}else if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				;
			}
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_STOPF);
	//4. Handle For interrupt generated by STOPF event
	// Note : Stop detection flag is applicable only slave mode . For master this flag will never be set
	if(temp1 && temp3)
	{
		//STOPF flag id set
		//clear STOP flag: read SR1 then write to CR1
		pI2CHandle->pI2Cx->I2C_CR1 |= 0x0000;

		//notify appplication STOP is detexted
		I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_STOP);
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_TXE);
	//5. Handle For interrupt generated by TXE event
	if(temp1 && temp2 && temp3)
	{
		//check for device mode(M/S)
		if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
		{
			//TXE flag id set
			//we have to do data transmission
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
			{
				I2C_MasterHandleTXEInterrupt(pI2CHandle);
			}
		}else
		{
			//slave
			//make sure slave is in transmitter mode
			if(pI2CHandle->pI2Cx->I2C_SR2 == I2C_SR2_TRA)
			{
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_REQ);
			}
		}
	}

	temp3 = pI2CHandle->pI2Cx->I2C_SR1 & (1 << I2C_SR1_RXNE);
	//6. Handle For interrupt generated by RXNE event
	if(temp1 && temp2 && temp3)
	{
		//check for device mode
		if(pI2CHandle->pI2Cx->I2C_SR2 & (1 << I2C_SR2_MSL))
		{
			//RXNE flag id set
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				I2C_MasterHandleRXNEInterrupt(pI2CHandle);
			}
		}else
		{
			//slave
			//make sure slave is in reciever mode
			if(pI2CHandle->pI2Cx->I2C_SR2 == I2C_SR2_TRA)
			{
				I2C_ApplicationEventCallback(pI2CHandle, I2C_EV_DATA_RCV);
			}
		}
	}
}

/*********************************************************************
 * @fn      		  - I2C_ER_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the code also define these macros in the driver
						header file
						#define I2C_ERROR_BERR  3
						#define I2C_ERROR_ARLO  4
						#define I2C_ERROR_AF    5
						#define I2C_ERROR_OVR   6
						#define I2C_ERROR_TIMEOUT 7

 */

void I2C_ER_IRQHandling(I2C_Handle_t *pI2CHandle)
{

	uint32_t temp1,temp2;

    //Know the status of  ITERREN control bit in the CR2
	temp2 = (pI2CHandle->pI2Cx->I2C_CR2) & ( 1 << I2C_CR2_ITERREN);


/***********************Check for Bus error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1<< I2C_SR1_BERR);
	if(temp1  && temp2 )
	{
		//This is Bus error

		//Implement the code to clear the buss error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_BERR);

		//Implement the code to notify the application about the error
	   I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_BERR);
	}

/***********************Check for arbitration lost error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_ARLO );
	if(temp1  && temp2)
	{
		//This is arbitration lost error

		//Implement the code to clear the arbitration lost error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_ARLO);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle,I2C_ERROR_ARLO);

	}

/***********************Check for ACK failure  error************************************/

	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_AF);
	if(temp1  && temp2)
	{
		//This is ACK failure error

	    //Implement the code to clear the ACK failure error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_AF);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_AF);

	}

/***********************Check for Overrun/underrun error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_OVR);
	if(temp1  && temp2)
	{
		//This is Overrun/underrun

	    //Implement the code to clear the Overrun/underrun error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_OVR);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_OVR);

	}

/***********************Check for Time out error************************************/
	temp1 = (pI2CHandle->pI2Cx->I2C_SR1) & ( 1 << I2C_SR1_TIMEOUT);
	if(temp1  && temp2)
	{
		//This is Time out error

	    //Implement the code to clear the Time out error flag
		pI2CHandle->pI2Cx->I2C_SR1 &= ~( 1 << I2C_SR1_TIMEOUT);
		//Implement the code to notify the application about the error
		I2C_ApplicationEventCallback(pI2CHandle, I2C_ERROR_TIMEOUT);
	}
}




