/*
 * stm32f407_gpio_driver.c
 *
 *  Created on: Aug 21, 2025
 *      Author: ACER1
 */
#include "stm32f407_gpio_driver.h"

/*  fn-     GPIO_PeriClockControl
 *
 *  brief-  This function enables or disables the peripheral clock for the given GPIO port
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral (GPIOA, GPIOB, etc.)
 *  param-  EnORDi: Enable or disable macro (ENABLE or DISABLE)
 *
 *  return- None
 */
void GPIO_PeriClockControl(GPIO_RegDef_t *pGPIOx, uint8_t EnORDi) {
    if (EnORDi == ENABLE) {
        if (pGPIOx == GPIOA) {
            GPIOA_PCLK_EN();
        } else if (pGPIOx == GPIOB) {
            GPIOB_PCLK_EN();
        } else if (pGPIOx == GPIOC) {
            GPIOC_PCLK_EN();
        } else if (pGPIOx == GPIOD) {
            GPIOD_PCLK_EN();
        } else if (pGPIOx == GPIOE) {
            GPIOE_PCLK_EN();
        } else if (pGPIOx == GPIOF) {
            GPIOF_PCLK_EN();
        } else if (pGPIOx == GPIOG) {
            GPIOG_PCLK_EN();
        } else if (pGPIOx == GPIOH) {
            GPIOH_PCLK_EN();
        } else if (pGPIOx == GPIOI) {
            GPIOI_PCLK_EN();
        }
    } else {
        if (pGPIOx == GPIOA) {
            GPIOA_PCLK_DI();
        } else if (pGPIOx == GPIOB) {
            GPIOB_PCLK_DI();
        } else if (pGPIOx == GPIOC) {
            GPIOC_PCLK_DI();
        } else if (pGPIOx == GPIOD) {
            GPIOD_PCLK_DI();
        } else if (pGPIOx == GPIOE) {
            GPIOE_PCLK_DI();
        } else if (pGPIOx == GPIOF) {
            GPIOF_PCLK_DI();
        } else if (pGPIOx == GPIOG) {
            GPIOG_PCLK_DI();
        } else if (pGPIOx == GPIOH) {
            GPIOH_PCLK_DI();
        } else if (pGPIOx == GPIOI) {
            GPIOI_PCLK_DI();
        }
    }
}

/*  fn-     GPIO_Init
 *
 *  brief-  Initializes a GPIO pin according to the specified parameters in the GPIO_Handle_t structure
 *          Configures mode, speed, pull-up/pull-down, output type, and alternate function
 *
 *  param-  pGPIOHandle: Pointer to GPIO handle structure containing pin configuration
 *
 *  return- None
 */
void GPIO_Init(GPIO_Handle_t *pGPIOHandle) {
    uint32_t temp = 0;

    //enable peripheral clk
    GPIO_PeriClockControl(pGPIOHandle->pGPIOx, ENABLE);

    //config mode
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode <= GPIO_MODE_ANALOG) {
        //non interrupt mode
        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode
                << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
        pGPIOHandle->pGPIOx->MODER &= ~(0x3
                << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); //clearing
        pGPIOHandle->pGPIOx->MODER |= temp; //setting
    } else {
        //interrupt mode
        if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_FT) {
            //configure FTSR (Falling Trigger)
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            //clear the corresponding RTSR bit
            EXTI->RTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RT) {
            //configure RTSR (Rising Trigger)
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            //clear the corresponding FTSR bit
            EXTI->FTSR &= ~(1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        } else if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_IT_RFT) {
            //configure both FTSR & RTSR (Rising/Falling Trigger)
            EXTI->RTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
            EXTI->FTSR |= (1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        }

        //configure the gpio port selection in SYSCFG_EXTICR
        uint8_t temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 4;
        uint8_t temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 4;
        uint8_t portcode = GPIOA_BASEADDR_TO_CODE(pGPIOHandle->pGPIOx);
        SYSCFG_PCLK_EN();
        SYSCFG->EXTICR[temp1] &= ~(0xF << (temp2 * 4)); // Clear existing setting
        SYSCFG->EXTICR[temp1] |= portcode << (temp2 * 4); // Set new port code

        //enable EXTI interrupt delivery using IMR
        EXTI->IMR |= 1 << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber;
    }
    temp = 0;

    //config speed
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinSpeed
            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->OSPEEDR &= ~(0x3 << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); // Clear
    pGPIOHandle->pGPIOx->OSPEEDR |= temp; // Set

    temp = 0;
    //config pupd
    temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinPuPdControl
            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber));
    pGPIOHandle->pGPIOx->PUPDR &= ~(0x3
            << (2 * pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber)); //clearing
    pGPIOHandle->pGPIOx->PUPDR |= temp; //setting

    temp = 0;
    //config optype
    if (pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_OUT ||
        pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN) {
        temp = (pGPIOHandle->GPIO_PinConfig.GPIO_PinOPType
                << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber);
        pGPIOHandle->pGPIOx->OTYPER &= ~(0x1
                << pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber); //clearing
        pGPIOHandle->pGPIOx->OTYPER |= temp; //setting
    }

    temp = 0;
    //config alt fn
    if ((pGPIOHandle->GPIO_PinConfig.GPIO_PinMode == GPIO_MODE_ALTFN)) {
        uint32_t temp1, temp2;
        temp1 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber / 8;
        temp2 = pGPIOHandle->GPIO_PinConfig.GPIO_PinNumber % 8;
        pGPIOHandle->pGPIOx->AFR[temp1] &= ~(0xF << (4 * temp2));
        pGPIOHandle->pGPIOx->AFR[temp1] |=
                (pGPIOHandle->GPIO_PinConfig.GPIO_PinAltFuncMode << (4 * temp2));
    }
}

/*  fn-     GPIO_DeInit
 *
 *  brief-  Resets all registers of the given GPIO port to their default values
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral to reset
 *
 *  return- None
 */
void GPIO_DeInit(GPIO_RegDef_t *pGPIOx) {
    if (pGPIOx == GPIOA) {
        GPIOA_REG_RESET();
    } else if (pGPIOx == GPIOB) {
        GPIOB_REG_RESET();
    } else if (pGPIOx == GPIOC) {
        GPIOC_REG_RESET();
    } else if (pGPIOx == GPIOD) {
        GPIOD_REG_RESET();
    } else if (pGPIOx == GPIOE) {
        GPIOE_REG_RESET();
    } else if (pGPIOx == GPIOF) {
        GPIOF_REG_RESET();
    } else if (pGPIOx == GPIOG) {
        GPIOG_REG_RESET();
    } else if (pGPIOx == GPIOH) {
        GPIOH_REG_RESET();
    } else if (pGPIOx == GPIOI) {
        GPIOI_REG_RESET();
    }
}

/*  fn-     GPIO_ReadFromInputPin
 *
 *  brief-  Reads the value from a specific input pin
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral
 *  param-  PinNumber: Pin number to read (0-15)
 *
 *  return- Value of the specified pin (0 or 1)
 */
uint8_t GPIO_ReadFromInputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) {
    uint8_t value;
    value = (uint8_t) ((pGPIOx->IDR >> PinNumber) & 0x00000001);
    return value;
}

/*  fn-     GPIO_ReadFromInputPort
 *
 *  brief-  Reads the value from the entire input port
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral
 *
 *  return- 16-bit value representing the state of all pins in the port
 */
uint16_t GPIO_ReadFromInputPort(GPIO_RegDef_t *pGPIOx) {
    uint16_t value;
    value = (uint16_t) pGPIOx->IDR;
    return value;
}

/*  fn-     GPIO_WriteToOutputPin
 *
 *  brief-  Writes a value to a specific output pin
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral
 *  param-  PinNumber: Pin number to write to (0-15)
 *  param-  Value: Value to write (GPIO_PIN_SET or GPIO_PIN_RESET)
 *
 *  return- None
 */
void GPIO_WriteToOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber,
        uint8_t Value) {
    if (Value == GPIO_PIN_SET) {
        //write to the output data register at the bit corresponding to the bit number
        pGPIOx->ODR |= (1 << PinNumber);
    } else {
        //write zero
        pGPIOx->ODR &= ~(1 << PinNumber);
    }
}

/*  fn-     GPIO_WriteToOutputPort
 *
 *  brief-  Writes a value to the entire output port
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral
 *  param-  Value: 16-bit value to write to the port
 *
 *  return- None
 */
void GPIO_WriteToOutputPort(GPIO_RegDef_t *pGPIOx, uint16_t Value) {
    pGPIOx->ODR = Value;
}

/*  fn-     GPIO_ToggleOutputPin
 *
 *  brief-  Toggles the state of a specific output pin
 *
 *  param-  pGPIOx: Base address of the GPIO peripheral
 *  param-  PinNumber: Pin number to toggle (0-15)
 *
 *  return- None
 */
void GPIO_ToggleOutputPin(GPIO_RegDef_t *pGPIOx, uint8_t PinNumber) {
    pGPIOx->ODR ^= (1 << PinNumber);
}

/*  fn-     GPIO_IRQInterruptConfig
 *
 *  brief-  Configures interrupt for the given IRQ number
 *
 *  param-  IRQNumber: Interrupt request number
 *  param-  IRQPriority: Interrupt priority level
 *  param-  EnOrDi: Enable or disable the interrupt
 *
 *  return- None
 */
void GPIO_IRQInterruptConfig(uint8_t IRQNumber, uint8_t IRQPriority,
        uint8_t EnOrDi) {
    if (EnOrDi == ENABLE) {
        if (IRQNumber <= 31) {
            //program ISER0 register
            *NVIC_ISER0 |= (1 << IRQNumber);
        } else if (IRQNumber > 31 && IRQNumber < 64) { //32 to 63
            //program ISER1 register
            *NVIC_ISER1 |= (1 << (IRQNumber % 32));
        } else if (IRQNumber >= 64 && IRQNumber < 96) {
            //program ISER2 register //64 to 95
            *NVIC_ISER2 |= (1 << (IRQNumber % 64));
        }
    } else {
        if (IRQNumber <= 31) {
            //program ICER0 register
            *NVIC_ICER0 = (1 << IRQNumber);  // Use assignment, not OR
        } else if (IRQNumber > 31 && IRQNumber < 64) {
            //program ICER1 register
            *NVIC_ICER1 = (1 << (IRQNumber % 32));  // Use assignment, not OR
        } else if (IRQNumber >= 64 && IRQNumber < 96) {
            //program ICER2 register
            *NVIC_ICER2 = (1 << (IRQNumber % 64));  // Use assignment, not OR
        }
    }
}

void GPIO_IRQ_PriorityConfig(uint8_t IRQNumber, uint8_t IRQPriority) {
    //first find ipr reg
    uint8_t iprx = IRQNumber / 4;
    uint8_t iprx_section = IRQNumber % 4;

    uint8_t shift_amount = (8 * iprx_section) + (8 - NO_PR_BITS_IMPLEMENTED);
    *(NVIC_PR_BASE_ADDR + iprx) &= ~(0xFF << (8 * iprx_section));  // Clear existing priority
    *(NVIC_PR_BASE_ADDR + iprx) |= (IRQPriority << shift_amount);  // Set new priority
}

/*  fn-     GPIO_IRQHandling
 *
 *  brief-  Handles the GPIO interrupt for the given pin number
 *
 *  param-  PinNumber: Pin number that triggered the interrupt
 *
 *  return- None
 */
void GPIO_IRQHandling(uint8_t PinNumber) {
    // Clear the EXTI pending register corresponding to pin number
    if (EXTI->PR & (1 << PinNumber)) {
        // Clear by writing 1 to the bit (as per STM32 reference manual)
        EXTI->PR = (1 << PinNumber);
    }
}
