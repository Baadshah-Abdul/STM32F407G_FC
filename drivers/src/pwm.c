/*
 * pwm.c
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */
#include "pwm.h"

uint8_t armed = 0;			//armed for flight or not

void PWM_GPIO_Config(void)
{
	GPIOD_PCLK_EN();
	TIM4_PCLK_EN();

	GPIO_Handle_t PWMpins;
	PWMpins.pGPIOx = GPIOD;
	PWMpins.GPIO_PinConfig.GPIO_PinMode = GPIO_MODE_ALTFN;
	PWMpins.GPIO_PinConfig.GPIO_PinAltFuncMode = 2;
	PWMpins.GPIO_PinConfig.GPIO_PinOPType = GPIO_OP_TYPE_PP;
	PWMpins.GPIO_PinConfig.GPIO_PinPuPdControl = GPIO_NO_PUPD;
	PWMpins.GPIO_PinConfig.GPIO_PinSpeed = GPIO_SPEED_FAST;

	PWMpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_12;
	GPIO_Init(&PWMpins);

	PWMpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_13;
	GPIO_Init(&PWMpins);

	PWMpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_14;
	GPIO_Init(&PWMpins);

	PWMpins.GPIO_PinConfig.GPIO_PinNumber = GPIO_PIN_NO_15;
	GPIO_Init(&PWMpins);
}

void PWM_TIM_Config(void)
{

	TIM_RegDef_t *pTIM4 = TIM4;
	uint32_t pclk = Get_PeriCLK();

	//Setup the Time Base
	pTIM4->PSC = (pclk/1000000) - 1;    // Prescalar (16MHz/1000000) - 1 = 1MHz
	//pTIM4->ARR = 2499;  // Period: 1MHz / 2500 = 400Hz frequency
	pTIM4->ARR = 19999;  // 1MHz / 20000 = 50Hz (20ms period) for standard ESCs


	pTIM4->CCMR1 |= (0x6 << 4) | (1 << 3);  // Channel 1
	pTIM4->CCMR1 |= (0x6 << 12) | (1 << 11); // Channel 2
	pTIM4->CCMR2 |= (0x6 << 4) | (1 << 3);  // Channel 3
	pTIM4->CCMR2 |= (0x6 << 12) | (1 << 11); // Channel 4

	//enable each channel to see the signal on the pins
	pTIM4->CCER |= (1 << 0) | (1 << 4) | (1 << 8) | (1 << 12);
	pTIM4->CR1 |= (1 << 7);
	pTIM4->CR1 |= (1 << 0); //start the counter!

}

void PWM_Arm(void)
{
	TIM4->CCR1 = PWM_MIN;
	TIM4->CCR2 = PWM_MIN;
	TIM4->CCR3 = PWM_MIN;
	TIM4->CCR4 = PWM_MIN;
	armed = 1;
}

void PWM_DisArm(void)
{
	TIM4->CCR1 = PWM_DISARM;
	TIM4->CCR2 = PWM_DISARM;
	TIM4->CCR3 = PWM_DISARM;
	TIM4->CCR4 = PWM_DISARM;
	armed = 0;

}

// Add this helper function at the top of pwm.c
float constrain(float value, float min, float max)
{
	if (value < min)
		return min;
	if (value > max)
		return max;
	return value;
}

void PWM_RP(float throttle, float pitch_correction, float roll_correction, float yaw_correction)
{
    if (armed)
    {
        float min_spin = PWM_MIN_SPIN;

        // Standard X-frame mixing:
        // M1 FL CW  → -pitch +roll +yaw
        // M2 FR CCW → -pitch -roll -yaw
        // M3 BL CCW → +pitch +roll -yaw
        // M4 BR CW  → +pitch -roll +yaw
        TIM4->CCR1 = (uint32_t)constrain(throttle - pitch_correction + roll_correction + yaw_correction, min_spin, PWM_MAX);

        TIM4->CCR2 = (uint32_t)constrain(throttle - pitch_correction - roll_correction - yaw_correction, min_spin, PWM_MAX);

        TIM4->CCR3 = (uint32_t)constrain(throttle + pitch_correction + roll_correction - yaw_correction, min_spin, PWM_MAX);

        TIM4->CCR4 = (uint32_t)constrain(throttle + pitch_correction - roll_correction + yaw_correction, min_spin, PWM_MAX);
    }
    else { PWM_DisArm(); }
}
