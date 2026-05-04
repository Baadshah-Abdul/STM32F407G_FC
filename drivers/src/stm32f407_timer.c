/*
 * stm32f407_timer.c
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */
#include "stm32f407_timer.h"

// Global variable to track system time in milliseconds
static volatile uint32_t msTicks = 0;

/***********************************************************************
 * @fn				-
 * @brief 			-
 * @param			-
 */
uint32_t Get_SYSCLK(void)
{
	// Bits 3:2 indicate the system clock source
	uint8_t clksrc = (RCC->CFGR >> 2) & 0x3;

	switch (clksrc)
	{
	case 0:
		return 16000000; // 00: HSI (Internal 16MHz)
	case 1:
		return 8000000;  // 01: HSE (External 8MHz)
	case 2:
		return 168000000; // 10: PLL (Assume max)
	default:
		return 16000000;
	}
}

/*********************************************************************
 * @fn      		  - SysTick_Init
 * @brief             - Initializes the SysTick timer for 1ms intervals
 * @param[in]         - ticks: Number of ticks between interrupts
 */
void SysTick_Init(uint32_t ticks)
{
	SYSTICK->LOAD = ticks - 1; /* Set reload value */
	SYSTICK->VAL = 0; /* Clear current value */
	SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT
			| SYSTICK_CTRL_ENABLE; /* Enable Timer & Interrupt */
}

// Automatically called by the processor every 1ms
void SysTick_Handler(void)
{
	msTicks++;
}

uint32_t get_ms(void)
{
	return msTicks;
}

void delay_ms(uint32_t ms)
{
	uint32_t start = get_ms();
	while ((get_ms() - start) < ms)
		;
}

/***********************************************************************
 * @fn				-
 * @brief 			-
 * @param			-
 */
uint32_t Get_PeriCLK(void)
{
	uint32_t pclk = Get_SYSCLK();
	uint32_t mask = ((1u << 3) - 1) << 10;  // 3 bits starting at bit 10
	uint32_t result = (RCC->CFGR & mask) >> 10;
	if (result == 0)
	{
		return pclk;
	}
	else
	{
		switch (result)
		{
		//0b to tell c im using binary
		case 0b100:
			return (pclk / 2);
			break;
		case 0b101:
			return (pclk / 4);
			break;
		case 0b110:
			return (pclk / 8);
			break;
		case 0b111:
			return (pclk / 16);
			break;
		}
	}
	return 0;
}

/********************************************************************************
 * @fn      		  - Timer_Init
 * @brief             - Configures general purpose timers (TIM2-TIM5)
 * @param[in]         - pTimerHandle: Pointer to configuration handle
 */
void Timer_Init(Timer_Handle_t *pTimerHandle)
{
	// Set the Prescaler (PSC) to slow down the clock
	pTimerHandle->pTIMx->PSC = pTimerHandle->TimerConfig.Timer_Prescaler;

	// Set the Auto-reload value (ARR) to define the frequency
	pTimerHandle->pTIMx->ARR = pTimerHandle->TimerConfig.Timer_Period;

	// Configure the counter mode (Up/Down)
	if (pTimerHandle->TimerConfig.Timer_Mode == 0)
	{ // Assuming 0 is Upcounter
		pTimerHandle->pTIMx->CR1 &= ~(1 << 4); // CMS bits
	}

	// Enable the counter
	pTimerHandle->pTIMx->CR1 |= (1 << 0); // CEN bit
}

/***************************REMEBER TO FIX**********************/
// For USART1, USART6 (APB2)
uint32_t Get_PCLK2(void)
{
	return 16000000;  // APB2 = 16MHz
}

// For USART2, USART3, UART4, UART5 (APB1)
uint32_t Get_PCLK1(void)
{
	return 16000000;  // APB1 = 16MHz
}

