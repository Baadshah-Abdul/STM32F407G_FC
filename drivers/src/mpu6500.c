/*
 * mpu6500.c
 *
 * Created on: Jan 7, 2026
 * Author: ACER1
 */

#include "mpu6500.h"
#include "stm32f407_timer.h"

//roll and pitch
#define RAD_TO_DEG 57.2957795f
#define DT 0.005f

void SPI1_GPIO_Config(void)
{
	GPIO_Handle_t SPIpins;
	SPIpins.pGPIOx = GPIOA;
	SPIpins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	SPIpins.GPIO_PinConfig.GPIO_PinAltFuncMode = 5;
	SPIpins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	SPIpins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	SPIpins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	SPIpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_5; // SCK
	GPIO_Init(&SPIpins);
	SPIpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_6; // MISO
	GPIO_Init(&SPIpins);
	SPIpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_7; // MOSI
	GPIO_Init(&SPIpins);
}

void SPI1_Config(void)
{
	SPI_Handle_t SPI1_handle;
	SPI1_handle.pSPIx = SPI1;
	SPI_PeriClockControl(SPI1, ENABLE);
	SPI1_handle.SPIConfig.SPI_BusConfig = SPI_BUS_CONFIG_FD;
	SPI1_handle.SPIConfig.SPI_DeviceMode = SPI_DEVICE_MODE_MASTER;

	/* Speed set to DIV8 (~2MHz) for cleaner data latching compared to DIV256 */
	SPI1_handle.SPIConfig.SPI_SclkSpeed  = SPI_SCLK_SPEED_DIV8;

	SPI1_handle.SPIConfig.SPI_CPOL = SPI_CPOL_LOW;
	SPI1_handle.SPIConfig.SPI_CPHA = SPI_CPHA_LOW;
	SPI1_handle.SPIConfig.SPI_SSM = SPI_SSM_EN;
	SPI_Init(&SPI1_handle);
	SPI_SSIConfig(SPI1, ENABLE);
}

void MPU6500_CS_Config(void)
{
	GPIO_Handle_t Mpu_CS_Pin;
	GPIO_PeriClockControl(GPIOA, ENABLE);
	Mpu_CS_Pin.pGPIOx = GPIOA;
	Mpu_CS_Pin.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	Mpu_CS_Pin.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_4;
	Mpu_CS_Pin.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	Mpu_CS_Pin.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;
	GPIO_Init(&Mpu_CS_Pin);
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);

	// PE3 Trick to disable onboard sensor
	GPIO_Handle_t Onboard_CS;
	GPIO_PeriClockControl(GPIOE, ENABLE);
	Onboard_CS.pGPIOx = GPIOE;
	Onboard_CS.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_3;
	Onboard_CS.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_OUT;
	GPIO_Init(&Onboard_CS);
	GPIO_WriteToOutputPin(GPIOE, GPIO_PIN_NO_3, GPIO_PIN_SET);
}

uint8_t MPU6500_Write(uint8_t reg_addr, uint8_t data)
{
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_RESET);
	SPI_TransmitReceive(SPI1, reg_addr);
	SPI_TransmitReceive(SPI1, data);
	SPI_BUSY_WAIT; // Use define from mpu6500.h
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);
	return 0;
}

uint8_t MPU6500_Read(uint8_t reg_addr)
{
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_RESET);
	SPI_TransmitReceive(SPI1, (uint8_t)(reg_addr | READ_BIT)); // Use READ_BIT define
	uint8_t Rxdata = SPI_TransmitReceive(SPI1, 0x00);
	SPI_BUSY_WAIT;
	GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);
	return Rxdata;
}

void MPU6500_Init(void)
{
	MPU6500_Write(PWR_MGMT_1, 0x80); // Reset
	for(volatile int i=0; i<2000000; i++);    // Wait for reset to complete
	MPU6500_Write(PWR_MGMT_1, 0x01); // Wakeup
	MPU6500_Write(USER_CTRL, 0x10);  // Disable I2C
	MPU6500_Write(GYRO_CONFIG, 0x18);// +/- 2000 deg/s
	MPU6500_Write(ACCEL_CONFIG, 0x10);// +/- 8g
	MPU6500_Write(CONFIG, 0x03);     // DLPF bandwidth 42Hz
}

void MPU6500_Read_RawData(int16_t* pAccel, int16_t* pGyro)
{
    uint8_t buffer[14];

    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_RESET);

    // Send register address and capture the dummy byte
    SPI_TransmitReceive(SPI1, (uint8_t)(ACCEL_OUT_H | READ_BIT));

    // Burst read 14 bytes (Accel, Temp, Gyro)
    for(int i = 0; i < 14; i++){
        buffer[i] = SPI_TransmitReceive(SPI1, 0x00);
    }

    SPI_BUSY_WAIT;
    GPIO_WriteToOutputPin(GPIOA, GPIO_PIN_NO_4, GPIO_PIN_SET);

    // Combine High and Low bytes
    pAccel[0] = (int16_t)((buffer[0] << 8) | buffer[1]);
    pAccel[1] = (int16_t)((buffer[2] << 8) | buffer[3]);
    pAccel[2] = (int16_t)((buffer[4] << 8) | buffer[5]);

    pGyro[0]  = (int16_t)((buffer[8] << 8) | buffer[9]);
    pGyro[1]  = (int16_t)((buffer[10] << 8) | buffer[11]);
    pGyro[2]  = (int16_t)((buffer[12] << 8) | buffer[13]);
}

//static MPU6500_FinalValue_t mpu_data;
MPU6500_FinalValue_t mpu_data;



void process_data(int16_t* raw_acc, int16_t* raw_gyro, float* off_acc, float* off_gyro) {
    // Accel to G's
    float ax_now = (raw_acc[0] - off_acc[0]) / 4096.0f;
    float ay_now = (raw_acc[1] - off_acc[1]) / 4096.0f;
    float az_now = (raw_acc[2] - off_acc[2]) / 4096.0f;

    // Gyro to Degrees/Second
    float gx_now = (raw_gyro[0] - off_gyro[0]) / 16.4f;
    float gy_now = (raw_gyro[1] - off_gyro[1]) / 16.4f;
    float gz_now = (raw_gyro[2] - off_gyro[2]) / 16.4f;

    float GYRO_ALPHA = 0.4f;
    float ACCEL_ALPHA = 0.03f;
    // Low Pass Filter (LPF)
    // Formula: Output = (Old * 0.95) + (New * 0.05)
    mpu_data.accel_f[0] = (mpu_data.accel_f[0] * (1.0f - ACCEL_ALPHA)) + (ax_now * ACCEL_ALPHA);
    mpu_data.accel_f[1] = (mpu_data.accel_f[1] * (1.0f - ACCEL_ALPHA)) + (ay_now * ACCEL_ALPHA);
    mpu_data.accel_f[2] = (mpu_data.accel_f[2] * (1.0f - ACCEL_ALPHA)) + (az_now * ACCEL_ALPHA);

    mpu_data.gyro_f[0] = (mpu_data.gyro_f[0] * (1.0f - GYRO_ALPHA)) + (gx_now * GYRO_ALPHA);
    mpu_data.gyro_f[1] = (mpu_data.gyro_f[1] * (1.0f - GYRO_ALPHA)) + (gy_now * GYRO_ALPHA);
    mpu_data.gyro_f[2] = (mpu_data.gyro_f[2] * (1.0f - GYRO_ALPHA)) + (gz_now * GYRO_ALPHA);
}

MPU6500_FinalValue_t* MPU6500_GetData(void) {
    return &mpu_data;
}

void MPU6500_Calibrate()
{
	    for(int i = 0; i < 1000; i++) {
	        MPU6500_Read_RawData(mpu_data.accel_raw, mpu_data.gyro_raw);
	        mpu_data.ax_sum += mpu_data.accel_raw[0];
	        mpu_data.ay_sum += mpu_data.accel_raw[1];
	        // We subtract 4096 (1g) so the offset targets '0' gravity for Z
	        mpu_data.az_sum += (mpu_data.accel_raw[2] - MPU6500_ACCEL_SENS_4G);
	        mpu_data.gx_sum += mpu_data.gyro_raw[0];
	        mpu_data.gy_sum += mpu_data.gyro_raw[1];
	        mpu_data.gz_sum += mpu_data.gyro_raw[2];

	        // Blink LED every 100 samples so you know it's working
	        if(i % 100 == 0) GPIO_ToggleOutputPin(GPIOD, GPIO_PIN_NO_12);

	        delay_ms(5);
	    }

	    mpu_data.ax_off = (float)mpu_data.ax_sum / 1000.0f;
	    mpu_data.ay_off = (float)mpu_data.ay_sum / 1000.0f;
	    mpu_data.az_off = (float)mpu_data.az_sum / 1000.0f;
	    mpu_data.gx_off = (float)mpu_data.gx_sum / 1000.0f;
	    mpu_data.gy_off = (float)mpu_data.gy_sum / 1000.0f;
	    mpu_data.gz_off = (float)mpu_data.gz_sum / 1000.0f;


}



void MPU6500_CalculateAngles(void) {
    // 1. Calculate Accelerometer Angles (The "Stable" but "Noisy" reference)
    float acc_roll  = atan2f(mpu_data.accel_f[1], mpu_data.accel_f[2]) * RAD_TO_DEG;
    float acc_pitch = atan2f(-mpu_data.accel_f[0],
                      sqrtf(mpu_data.accel_f[1] * mpu_data.accel_f[1] +
                            mpu_data.accel_f[2] * mpu_data.accel_f[2])) * RAD_TO_DEG;

    // 2. Complementary Filter (The "Secret Sauce")
    // Formula: Angle = 0.98 * (Angle + Gyro*DT) + 0.02 * (AccelAngle)
    // 0.98 trusts the gyro for short-term changes
    // 0.02 uses the accelerometer to fix long-term drift

    mpu_data.roll  = 0.98f * (mpu_data.roll  + mpu_data.gyro_f[0] * DT) + 0.02f * acc_roll;
    mpu_data.pitch = 0.98f * (mpu_data.pitch + mpu_data.gyro_f[1] * DT) + 0.02f * acc_pitch;
}









