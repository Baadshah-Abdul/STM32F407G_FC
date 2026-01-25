#include <stdio.h>
#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_spi_driver.h"
#include "mpu6500.h"
#include "stm32f407_timer.h"
#include "pwm.h"
#include "pid.h"

int main()
{
	// Enable FPU: Set CP10 and CP11 Full Access
	// 3 shifted to positions 20 and 22 is 15728640 (0xF00000)
	__vo uint32_t *pCPACR = (uint32_t*) 0xE000ED88;
	*pCPACR |= 0xF00000;

	//start timer4
	SysTick_Init(16000);

	//setup of mpu6500
	SPI1_GPIO_Config();
	SPI1_Config();
	MPU6500_CS_Config();

	SPI_PeripheralControl(SPI1, ENABLE);
	MPU6500_Init();

	uint8_t whoami = MPU6500_Read(MPU_WHO_AM_I);
	if (whoami == 0x70)
		printf("Communication success!!. 0x%x\n", whoami);		//%x for hex

	printf("Calibrating... Keep the board still!\n");

	MPU6500_FinalValue_t *sensor = MPU6500_GetData();
	MPU6500_Calibrate();

	//Initialize with first real values to avoid 0.00 spike
	sensor->accel_f[2] = 1.0f;

	uint32_t last_time = get_ms();
	int print_divider = 0;

	PWM_GPIO_Config();

	PWM_TIM_Config();
	printf("now\n");

	PWM_Arm();
	delay_ms(9000);

	float base_throttle = 1150.0f;
	//pid struct
	PID_Config_t pidPitch =
	{ .Kp = 1.0f, .Ki = 0.00f, .Kd = 0.01f, .output_limit = 400.0f };
	PID_Config_t pidRoll =
	{ .Kp = 1.0f, .Ki = 0.00f, .Kd = 0.01f, .output_limit = 400.0f };
	//delta time set to 5ms else if inconsistent I & D will increase
	float dt = 0.005f;

	uint32_t safety_counter = 0;

	while (1)
	{
		// Run the loop every 5ms (200Hz)
		if ((get_ms() - last_time) >= 5)
		{
			last_time = get_ms();

			MPU6500_Read_RawData(sensor->accel_raw, sensor->gyro_raw);
			float acc_offsets[3] =
			{ sensor->ax_off, sensor->ay_off, sensor->az_off };
			float gyro_offsets[3] =
			{ sensor->gx_off, sensor->gy_off, sensor->gz_off };
			process_data(sensor->accel_raw, sensor->gyro_raw, acc_offsets,
					gyro_offsets);

			MPU6500_CalculateAngles();

			// PID
			float pitch_corr = PID_Compute(&pidPitch, 0.0f, sensor->pitch, dt);
			float roll_corr = PID_Compute(&pidRoll, 0.0f, sensor->roll, dt);

			PWM_RP(base_throttle, pitch_corr, roll_corr);

			safety_counter++;
			if (safety_counter >= 6000)
			{
				if (armed == 1)
				{
					printf("Safety limit reached. Disarming motors.\n");
				}
				PWM_DisArm();
				pidPitch.Ki = 0.0f;
				pidRoll.Ki = 0.0f;
				safety_counter = 6001;
			}

			// Only print every 20th loop
			if (print_divider++ >= 20)
			{
				printf("Roll: %.2f | Pitch: %.2f\n", sensor->roll,
						sensor->pitch);
				print_divider = 0;
			}
		}
	}
}
