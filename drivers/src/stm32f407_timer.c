/*
 * stm32f407_timer.c
 *
 *  Created on: Jan 12, 2026
 *      Author: ACER1
 */
#include "stm32f407_timer.h"

// Global variable to track system time in milliseconds
static volatile uint32_t msTicks = 0;

/*********************************************************************
 * @fn      		  - SysTick_Init
 * @brief             - Initializes the SysTick timer for 1ms intervals
 * @param[in]         - ticks: Number of ticks between interrupts
 */
void SysTick_Init(uint32_t ticks) {
    SYSTICK->LOAD = ticks - 1;              /* Set reload value */
    SYSTICK->VAL  = 0;                      /* Clear current value */
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE |
                    SYSTICK_CTRL_TICKINT   |
                    SYSTICK_CTRL_ENABLE;    /* Enable Timer & Interrupt */
}

// The ISR: Automatically called by the processor every 1ms
void SysTick_Handler(void) {
    msTicks++;
}

uint32_t get_ms(void) {
    return msTicks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = get_ms();
    while((get_ms() - start) < ms);
}

/*********************************************************************
 * @fn      		  - Timer_Init
 * @brief             - Configures general purpose timers (TIM2-TIM5)
 * @param[in]         - pTimerHandle: Pointer to configuration handle
 */
void Timer_Init(Timer_Handle_t *pTimerHandle) {
    // 1. Set the Prescaler (PSC) to slow down the clock
    pTimerHandle->pTIMx->PSC = pTimerHandle->TimerConfig.Timer_Prescaler;

    // 2. Set the Auto-reload value (ARR) to define the frequency
    pTimerHandle->pTIMx->ARR = pTimerHandle->TimerConfig.Timer_Period;

    // 3. Configure the counter mode (Up/Down)
    if(pTimerHandle->TimerConfig.Timer_Mode == 0) { // Assuming 0 is Upcounter
        pTimerHandle->pTIMx->CR1 &= ~(1 << 4); // CMS bits
    }

    // 4. Enable the counter
    pTimerHandle->pTIMx->CR1 |= (1 << 0); // CEN bit
}

