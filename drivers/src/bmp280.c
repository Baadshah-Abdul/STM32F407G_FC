/**
 * bmp280.c
 * BMP280 Digital Pressure Sensor Driver for STM32F407
 *
 * Created on: May 2026
 * Author: ACER1
 *
 * Notes:
 * - Uses Forced Mode (single measurement, returns to sleep)
 * - Pressure x4 oversampling, Temperature x1 (minimal for compensation)
 * - Optimized for drone altitude estimation
 */

#include "bmp280.h"
#include "stm32f407_timer.h"

I2C_Handle_t I2C1Handle;

uint8_t recivebuf[32];

/* ==========================================================================
   I2C Peripheral Setup (PB10 = SCL, PB11 = SDA)
   ========================================================================== */
void I2C1_GPIO_Config(void)
{
    GPIO_Handle_t I2Cpins;
    I2Cpins.pGPIOx = GPIOB;
    I2Cpins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
    I2Cpins.GPIO_PinConfig.GPIO_PinAltFuncMode = 4;
    I2Cpins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_OD;
    I2Cpins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_PIN_PU;
    I2Cpins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

    I2Cpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6; /* SCL */
    GPIO_Init(&I2Cpins);
    I2Cpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7; /* SDA */
    GPIO_Init(&I2Cpins);
}



void I2C1_Config(void)
{
    I2C1Handle.pI2Cx = I2C1;
    I2C1Handle.I2CConfig.I2C_AckControl = I2C_ACK_ENABLE;
    I2C1Handle.I2CConfig.I2C_DeviceAddress = 0x01;
    I2C1Handle.I2CConfig.I2C_FMDutyCycle = I2C_FM_DUTY_2;
    I2C1Handle.I2CConfig.I2C_SCLSpeed = I2C_SCL_SPEED_SM;
    I2C_Init(&I2C1Handle);
}

void BMP280_Read_RawData(void)
{
	uint8_t commandcode, len;

	I2C1_GPIO_Config();
	I2C1_Config();
	//enable the i2c peripheral
	I2C_PeripheralControl(I2C1Handle, ENABLE);
	//ack bit is made 1 after PE=1
	I2C_ManageAcking(I2C1, ENABLE);

	while(1)
	{
		commandcode = 0x51;

		I2C_MasterSendData(&I2C1Handle, &commandcode, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);

		I2C_MasterReceiveData(&I2C1Handle, &len, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);

		commandcode = 0x52;

		I2C_MasterSendData(&I2C1Handle, &commandcode, 1, BMP280_I2C_ADDR, I2C_ENABLE_SR);

		I2C_MasterSendData(&I2C1Handle, recivebuf, len, BMP280_I2C_ADDR, I2C_DISABLE_SR);

		recivebuf[len+1] = '\0';
	}
}
