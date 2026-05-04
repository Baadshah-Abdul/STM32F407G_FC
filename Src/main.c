#include <stdio.h>
#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_spi_driver.h"
#include "mpu6500.h"
#include "stm32f407_timer.h"
#include "pwm.h"
#include "pid.h"
#include "crsf.h"



int main()
{
	// FPU enable
	// Enable FPU: Set CP10 and CP11 Full Access
	//	// 3 shifted to positions 20 and 22 is 15728640 (0xF00000)
	__vo uint32_t *pCPACR = (uint32_t*) 0xE000ED88;
	*pCPACR |= 0xF00000;

	// SysTick_Init
	uint32_t system_speed = Get_SYSCLK();
	SysTick_Init(system_speed/1000);

	// replace with: call CRSF_Init() here
	// reason: CRSF must be ready before the main loop starts
	CRSF_Init();


	printf("SYSCLK: %lu Hz\n", system_speed);
	printf("PeriCLK: %lu Hz\n", Get_PeriCLK());

	// SPI + MPU setup — no change
	//setup of mpu6500
	SPI1_GPIO_Config();
	SPI1_Config();
	MPU6500_CS_Config();
	SPI_PeripheralControl(SPI1, ENABLE);
	MPU6500_Init();
	printf("MPU init done\n");
	uint8_t whoami = MPU6500_Read(MPU_WHO_AM_I);
	if (whoami == 0x70)
		printf("Communication success!!. 0x%x\n", whoami);		//%x for hex

	printf("Calibrating... Keep the board still!\n");

	MPU6500_FinalValue_t *sensor = MPU6500_GetData();

	MPU6500_Calibrate();

	MPU6500_InitialiseAngles();

	PWM_GPIO_Config();
	PWM_TIM_Config();

	// remove PWM_Arm() and delay_ms(9000) from here
	// reason: arming is now controlled by the RC arm switch (CH5)
	//         drone must not arm automatically at startup

	// remove safety_counter and safety_triggered variables
	// reason: replaced entirely by CH5 arm switch from CRSF

	// add: get pointer to CRSF channel data
	CRSF_Data_t *rc = CRSF_GetChannels();

	float base_throttle = 1000.0f;
	//pid struct
	PID_Config_t pidPitch =
	{ .Kp=1.0f, .Ki=0.0f, .Kd=0.01f, .output_limit=400.0f, .i_limit=150.0f };
	PID_Config_t pidRoll  =
	{ .Kp=1.0f, .Ki=0.0f, .Kd=0.01f, .output_limit=400.0f, .i_limit=150.0f };
	PID_Config_t pidYaw   =
	{ .Kp=2.0f, .Ki=0.0f, .Kd=0.005f,.output_limit=400.0f, .i_limit=50.0f  };

	//delta time set to 5ms else if inconsistent I & D will increase
	float dt = 0.005f;

	uint32_t last_time = get_ms();
	int print_divider = 0;

	while (1)
	{
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

			// ── RC ARM SWITCH ────────────────────────────
			// check if rc->valid is 1 (fresh packet received)
			// AND check if CH5 (rc->ch[4]) converted to µs > 1500
			//     → this means arm switch is flipped ON
			// if both true AND not currently armed
			//     → call PWM_Arm()
			// if CH5 µs <= 1500 AND currently armed
			//     → call PWM_DisArm()
			//     → reset all three PID integrals to 0.0f
			if(rc->valid == 1)
			{
				if( CRSF_ToUs(rc->ch[4]) > 1500 && !armed)
				{
					PWM_Arm();
				}else if(CRSF_ToUs(rc->ch[4]) <= 1500)
				{
					PWM_DisArm();
					pidPitch.integral  = 0.0f;
					pidPitch.prev_error = 0.0f;
					pidRoll.integral   = 0.0f;
					pidRoll.prev_error  = 0.0f;
					pidYaw.integral    = 0.0f;
					pidYaw.prev_error   = 0.0f;
				}
			}

			// ── THROTTLE FROM RC ─────────────────────────
			// reason: CH3 is throttle stick from RadioMaster Pocket
			base_throttle = CRSF_ToUs(rc->ch[2]);

			// ── PID SETPOINTS FROM RC ────────────────────
			// replace 0.0f setpoint in pidRoll compute
			// with a mapped roll setpoint from rc->ch[0]
			// hint: CRSF_ToUs gives 1000-2000, centre is 1500
			//       subtract 1500 to get -500 to +500
			//       divide by a scale factor to get degrees
			//       suggested scale: divide by 50 → ±10 degrees max

			// replace 0.0f setpoint in pidPitch compute
			// with mapped pitch setpoint from rc->ch[1]
			// same scaling as roll

			// replace 0.0f setpoint in pidYaw compute
			// with mapped yaw rate setpoint from rc->ch[3]
			// same scaling as roll and pitch
			float roll_sp  = ((float)CRSF_ToUs(rc->ch[0]) - 1500.0f) / 50.0f;
			float pitch_sp = ((float)CRSF_ToUs(rc->ch[1]) - 1500.0f) / 50.0f;
			float yaw_sp   = ((float)CRSF_ToUs(rc->ch[3]) - 1500.0f) / 50.0f;

			float pitch_corr = PID_Compute(&pidPitch, pitch_sp, sensor->pitch, dt);
			float roll_corr  = PID_Compute(&pidRoll,  roll_sp,  sensor->roll,  dt);
			float yaw_corr   = PID_Compute(&pidYaw,   yaw_sp,   sensor->gyro_f[2], dt);

			PWM_RP(base_throttle, pitch_corr, roll_corr, yaw_corr);

			// if not armed, call PWM_DisArm() as failsafe
			// in case valid flag goes false (signal lost)
			if(!armed)
			{
				PWM_DisArm();
			}

			// Only print every 20th loop
			if (print_divider++ >= 20)
			{
				printf("Roll: %.2f Pitch: %.2f, Throttle: %d\n",
						sensor->roll, sensor->pitch,rc->ch[4]);
				print_divider = 0;
			}
		}
	}
}


