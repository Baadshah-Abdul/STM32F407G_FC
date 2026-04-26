/*
 * stm32f407_timer.h
 *
 * Created on: Jan 12, 2026
 * Author: ACER1
 */

#ifndef INC_STM32F407_TIMER_H_
#define INC_STM32F407_TIMER_H_

#include "stm32f407.h"

//SYSTICK REGISTER DEFINITIONS

typedef struct
{
	__vo uint32_t CTRL;    // Control and Status Register
	__vo uint32_t LOAD;    // Reload Value Register
	__vo uint32_t VAL;     // Current Value Register
	__vo uint32_t CALIB;   // Calibration Register
} SysTick_RegDef_t;

#define SYSTICK_BASE        (0xE000E010)
#define SYSTICK             ((SysTick_RegDef_t *)SYSTICK_BASE)

// SysTick Bit Positions
#define SYSTICK_CTRL_ENABLE    1     // Bit 0: 1 shifted by 0
#define SYSTICK_CTRL_TICKINT   2     // Bit 1: 1 shifted by 1
#define SYSTICK_CTRL_CLKSOURCE 4     // Bit 2: 1 shifted by 2

// TIMER CONFIGURATION STRUCTURE
// To initialize any general-purpose timer
typedef struct
{
	uint32_t Timer_Prescaler;
	uint32_t Timer_Period;      // Auto-reload value (ARR)
	uint32_t Timer_Mode;        // Upcounter/Downcounter
} Timer_Config_t;

typedef struct
{
	TIM_RegDef_t *pTIMx;        // Base address of the timer
	Timer_Config_t TimerConfig;
} Timer_Handle_t;

// Get the actual frequency and divide by 1000 to get ticks per millisecond
uint32_t Get_SYSCLK(void);
void SysTick_Init(uint32_t ticks);
uint32_t get_ms(void);
void delay_ms(uint32_t ms);
uint32_t Get_PeriCLK(void);
void Timer_Init(Timer_Handle_t *pTimerHandle);

#endif /* INC_STM32F407_TIMER_H_ */
