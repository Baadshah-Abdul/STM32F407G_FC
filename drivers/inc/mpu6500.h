/*
 * mpu6500.h
 *
 *  Created on: Jan 7, 2026
 *      Author: ACER1
 */

#ifndef INC_MPU6500_H_
#define INC_MPU6500_H_

#include <stdint.h>
#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_spi_driver.h"
#include <math.h>

//mpu6500 reg add
#define CONFIG				0x1A
#define MPU_WHO_AM_I		0x75
#define PWR_MGMT_1			0x6B
#define MPU_PWR_MGMT_2		0x6C
#define USER_CTRL			0x6A
#define SMPLRT_DIV			0x19
#define GYRO_CONFIG 		0x1B
#define ACCEL_CONFIG		0x1C
#define ACCEL_CONFIG_2		0x1D
#define INT_PIN_CONFIG		0x37
#define INT_ENABLE			0x38
#define INT_STATUS			0x3A
#define ACCEL_OUT_H			0x3B
#define GYRO_OUT_H			0x43
#define FIFO_ENABLE			0x23
#define READ_BIT          	0x80
#define SPI_BUSY_WAIT     while(SPI_GetFlagStatus(SPI1, SPI_BUSY_FLAG))

#define MPU6500_ACCEL_SENS_4G		4096.0f
#define MPU6500_GYRO_SENS_			16.4f
#define ALPHA						0.9996f

#define FILTER_SIZE 				4 //8 prev

typedef struct
{
	float accel_f[3];
	float gyro_f[3];
	int32_t ax_sum;
	int32_t ay_sum;
	int32_t az_sum;
	int32_t gx_sum;
	int32_t gy_sum;
	int32_t gz_sum;
	float ax_off;			// offsets
	float ay_off;
	float az_off;
	float gx_off;
	float gy_off;
	float gz_off;
	int16_t accel_raw[3];
	int16_t gyro_raw[3];
	float roll;  			// Degrees
	float pitch; 			// Degrees

} MPU6500_FinalValue_t;

// Peripheral Initializations
void SPI1_GPIO_Config(void);  // Setup PA5, PA6, PA7 for SPI
void SPI1_Config(void);       // Setup the SPI1 peripheral using your driver
void MPU6500_CS_Config(void); // Setup a GPIO (like PA4) as the Chip Select

uint8_t MPU6500_Write(uint8_t reg_addr, uint8_t data);
uint8_t MPU6500_Read(uint8_t reg_addr);

// MPU6500 Specific
void MPU6500_Init(void);
void MPU6500_Read_RawData(int16_t *pAccel, int16_t *pGyro);

void process_data(int16_t *raw_acc, int16_t *raw_gyro, float *off_acc,
		float *off_gyro);
MPU6500_FinalValue_t* MPU6500_GetData(void);
void MPU6500_Calibrate();

void MPU6500_CalculateAngles(void);

#endif /* INC_MPU6500_H_ */
