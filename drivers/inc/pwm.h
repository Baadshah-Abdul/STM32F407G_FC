/*
 * pwn.h
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */

#ifndef INC_PWM_H_
#define INC_PWM_H_

#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_timer.h"

#define PWM_MIN 1000
#define PWM_MAX 2000
#define PWM_ARM 900

void PWM_GPIO_Config(void);

void PWM_TIM_Config(void);

void PWM_DisArm(void);

extern uint8_t armed; //arming motors
void PWM_Arm(void);
void PWM_RP(float throttle, float pitch_correction, float roll_correction);

#endif /* INC_PWM_H_ */
