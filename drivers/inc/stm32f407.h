/*
 * stm32f407.h
 *
 *  Created on: Aug 19, 2025
 *      Author: ACER1
 */

#ifndef INC_STM32F407_H_
#define INC_STM32F407_H_

#include <stdint.h>  // Added for standard types

#define __vo volatile
#define __weak __attribute__((weak))

//******************processor specific details

//nvic iserx

#define NVIC_ISER0          ( (__vo uint32_t*)0xE000E100 )
#define NVIC_ISER1          ( (__vo uint32_t*)0xE000E104 )
#define NVIC_ISER2          ( (__vo uint32_t*)0xE000E108 )
#define NVIC_ISER3          ( (__vo uint32_t*)0xE000E10C )

//nvic icerx
#define NVIC_ICER0 			((__vo uint32_t*)0XE000E180)
#define NVIC_ICER1			((__vo uint32_t*)0XE000E184)
#define NVIC_ICER2  		((__vo uint32_t*)0XE000E188)
#define NVIC_ICER3			((__vo uint32_t*)0XE000E18C)

//priority register address calculation
#define NVIC_PR_BASE_ADDR 	((__vo uint32_t*)0xE000E400)

#define NO_PR_BITS_IMPLEMENTED 	4

// Base addresses of memory regions
#define FLASH_BASEADDR          0x08000000U
#define SRAM1_BASEADDR          0x20000000U  // 112KB
#define SRAM2_BASEADDR          0x2001C000U  // 16KB (corrected address)
#define ROM_BASEADDR            0x1FFF0000U
#define SRAM                    SRAM1_BASEADDR

// Peripheral bus base addresses
#define PERIPH_BASEADDR         0x40000000U
#define APB1PERIPH_BASEADDR     (PERIPH_BASEADDR + 0x00000)
#define APB2PERIPH_BASEADDR     (PERIPH_BASEADDR + 0x10000)
#define AHB1PERIPH_BASEADDR     (PERIPH_BASEADDR + 0x20000)
#define AHB2PERIPH_BASEADDR     0x50000000U

// Base addresses of peripherals on AHB1 bus
#define GPIOA_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0000)
#define GPIOB_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0400)
#define GPIOC_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0800)
#define GPIOD_BASEADDR          (AHB1PERIPH_BASEADDR + 0x0C00)
#define GPIOE_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1000)
#define GPIOF_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1400)
#define GPIOG_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1800)
#define GPIOH_BASEADDR          (AHB1PERIPH_BASEADDR + 0x1C00)
#define GPIOI_BASEADDR          (AHB1PERIPH_BASEADDR + 0x2000)

#define RCC_BASEADDR            (AHB1PERIPH_BASEADDR + 0x3800)

// Base addresses of peripherals on APB1 bus
#define I2C1_BASEADDR           (APB1PERIPH_BASEADDR + 0x5400)
#define I2C2_BASEADDR           (APB1PERIPH_BASEADDR + 0x5800)
#define I2C3_BASEADDR           (APB1PERIPH_BASEADDR + 0x5C00)

#define UART4_BASEADDR          (APB1PERIPH_BASEADDR + 0x4C00)
#define UART5_BASEADDR          (APB1PERIPH_BASEADDR + 0x5000)

#define USART2_BASEADDR         (APB1PERIPH_BASEADDR + 0x4400)
#define USART3_BASEADDR         (APB1PERIPH_BASEADDR + 0x4800)

#define SPI2_BASEADDR           (APB1PERIPH_BASEADDR + 0x3800)
#define SPI3_BASEADDR           (APB1PERIPH_BASEADDR + 0x3C00)

// Base addresses of peripherals on APB2 bus
#define SPI1_BASEADDR           (APB2PERIPH_BASEADDR + 0x3000)
#define USART1_BASEADDR         (APB2PERIPH_BASEADDR + 0x1000)
#define USART6_BASEADDR         (APB2PERIPH_BASEADDR + 0x1400)
#define EXTI_BASEADDR           (APB2PERIPH_BASEADDR + 0x3C00)
#define SYSCFG_BASEADDR         (APB2PERIPH_BASEADDR + 0x3800)

// GPIO Register Structure
typedef struct
{
	__vo uint32_t MODER;     // 0x00 - GPIO port mode register
	__vo uint32_t OTYPER;    // 0x04 - GPIO port output type register
	__vo uint32_t OSPEEDR;   // 0x08 - GPIO port output speed register
	__vo uint32_t PUPDR;     // 0x0C - GPIO port pull-up/pull-down register
	__vo uint32_t IDR;       // 0x10 - GPIO port input data register
	__vo uint32_t ODR;       // 0x14 - GPIO port output data register
	__vo uint32_t BSRR;      // 0x18 - GPIO port bit set/reset register
	__vo uint32_t LCKR;      // 0x1C - GPIO port configuration lock register
	__vo uint32_t AFR[2];    // 0x20-0x24 - GPIO alternate function registers
} GPIO_RegDef_t;

// SPI Register Structure
typedef struct
{
	__vo uint32_t CR1;        // 0x00 - SPI control register 1
	__vo uint32_t CR2;        // 0x04 - SPI control register 2
	__vo uint32_t SR;         // 0x08 - SPI status register
	__vo uint32_t DR;         // 0x0C - SPI data register
	__vo uint32_t CRCPR;      // 0x10 - SPI CRC polynomial register
	__vo uint32_t RXCRCR;     // 0x14 - SPI RX CRC register
	__vo uint32_t TXCRCR;     // 0x18 - SPI TX CRC register
	__vo uint32_t I2SCFGR;    // 0x1C - SPI_I2S configuration register
	__vo uint32_t I2SPR;      // 0x20 - SPI_I2S prescaler register
} SPI_RegDef_t;

typedef struct
{
	__vo uint32_t I2C_CR1;			//0x00 - I2C Control register 1
	__vo uint32_t I2C_CR2;			//0x04 - I2C Control register 2
	__vo uint32_t I2C_OAR1;			//0x08 - I2C Own address register 1
	__vo uint32_t I2C_OAR2;			//0x0C - I2C Own address register 2
	__vo uint32_t I2C_DR;			//0x10 - I2C Data register
	__vo uint32_t I2C_SR1;			//0x14 - I2C Status register 1
	__vo uint32_t I2C_SR2;			//0x18 - I2C Status register 2
	__vo uint32_t I2C_CCR;			//0x1C - I2C Clock control register
	__vo uint32_t I2C_TRISE;		//0x20 - I2C TRISE register
} I2C_RegDef_t;

// GPIO Peripheral Definitions
#define GPIOA   ((GPIO_RegDef_t*)GPIOA_BASEADDR)
#define GPIOB   ((GPIO_RegDef_t*)GPIOB_BASEADDR)
#define GPIOC   ((GPIO_RegDef_t*)GPIOC_BASEADDR)
#define GPIOD   ((GPIO_RegDef_t*)GPIOD_BASEADDR)
#define GPIOE   ((GPIO_RegDef_t*)GPIOE_BASEADDR)
#define GPIOF   ((GPIO_RegDef_t*)GPIOF_BASEADDR)
#define GPIOG   ((GPIO_RegDef_t*)GPIOG_BASEADDR)
#define GPIOH   ((GPIO_RegDef_t*)GPIOH_BASEADDR)
#define GPIOI   ((GPIO_RegDef_t*)GPIOI_BASEADDR)

// SPI Peripheral Definitions
#define SPI1  				((SPI_RegDef_t*)SPI1_BASEADDR)
#define SPI2  				((SPI_RegDef_t*)SPI2_BASEADDR)
#define SPI3  				((SPI_RegDef_t*)SPI3_BASEADDR)

// I2C Peripheral Definitions
#define I2C1	((I2C_RegDef_t*)I2C1_BASEADDR)
#define I2C2	((I2C_RegDef_t*)I2C2_BASEADDR)
#define I2C3	((I2C_RegDef_t*)I2C3_BASEADDR)

// RCC Register Structure
typedef struct
{
	__vo uint32_t CR;             // 0x00
	__vo uint32_t PLLCFGR;        // 0x04
	__vo uint32_t CFGR;           // 0x08
	__vo uint32_t CIR;            // 0x0C
	__vo uint32_t AHB1RSTR;       // 0x10
	__vo uint32_t AHB2RSTR;       // 0x14
	__vo uint32_t AHB3RSTR;       // 0x18
	uint32_t RESERVED0;      // 0x1C
	__vo uint32_t APB1RSTR;       // 0x20
	__vo uint32_t APB2RSTR;       // 0x24
	uint32_t RESERVED1[2];   // 0x28-0x2C
	__vo uint32_t AHB1ENR;        // 0x30
	__vo uint32_t AHB2ENR;        // 0x34
	__vo uint32_t AHB3ENR;        // 0x38
	uint32_t RESERVED2;      // 0x3C
	__vo uint32_t APB1ENR;        // 0x40
	__vo uint32_t APB2ENR;        // 0x44
	uint32_t RESERVED3[2];   // 0x48-0x4C
	__vo uint32_t AHB1LPENR;      // 0x50
	__vo uint32_t AHB2LPENR;      // 0x54
	__vo uint32_t AHB3LPENR;      // 0x58
	uint32_t RESERVED4;      // 0x5C
	__vo uint32_t APB1LPENR;      // 0x60
	__vo uint32_t APB2LPENR;      // 0x64
	uint32_t RESERVED5[2];   // 0x68-0x6C
	__vo uint32_t BDCR;           // 0x70
	__vo uint32_t CSR;            // 0x74
	uint32_t RESERVED6[2];   // 0x78-0x7C
	__vo uint32_t SSCGR;          // 0x80
	__vo uint32_t PLLI2SCFGR;     // 0x84
	__vo uint32_t PLLSAICFGR;     // 0x88
	__vo uint32_t DCKCFGR;        // 0x8C
	__vo uint32_t CKGATENR;       // 0x90
	__vo uint32_t DCKCFGR2;       // 0x94
} RCC_RegDef_t;

//EXTI register structure
typedef struct
{									//OFFSET
	__vo uint32_t IMR;				//0x00
	__vo uint32_t EMR;				//0x04
	__vo uint32_t RTSR;				//0x08
	__vo uint32_t FTSR;				//0x0C
	__vo uint32_t SWIER;			//0x10
	__vo uint32_t PR;				//0x14
} EXTI_RegDef_t;

typedef struct
{									//OFFSET
	__vo uint32_t MEMRMP;			//0x00
	__vo uint32_t PMC;				//0x04
	__vo uint32_t EXTICR[4];		//0x08-0x14
	uint32_t RESERVED1[2];			//0x18-0x1C
	__vo uint32_t CMPCR;			//0x20
	uint32_t RESERVED2[2];			//0x24-0x28
	__vo uint32_t CFGR;				//0x2C
} SYSCFG_RegDef_t;

#define RCC     ((RCC_RegDef_t*)RCC_BASEADDR)

#define EXTI 	((EXTI_RegDef_t*)EXTI_BASEADDR)

#define SYSCFG 	((SYSCFG_RegDef_t*)SYSCFG_BASEADDR)

// Clock enable macros for GPIOx peripherals
#define GPIOA_PCLK_EN()     (RCC->AHB1ENR |= (1 << 0))
#define GPIOB_PCLK_EN()     (RCC->AHB1ENR |= (1 << 1))
#define GPIOC_PCLK_EN()     (RCC->AHB1ENR |= (1 << 2))
#define GPIOD_PCLK_EN()     (RCC->AHB1ENR |= (1 << 3))
#define GPIOE_PCLK_EN()     (RCC->AHB1ENR |= (1 << 4))
#define GPIOF_PCLK_EN()     (RCC->AHB1ENR |= (1 << 5))
#define GPIOG_PCLK_EN()     (RCC->AHB1ENR |= (1 << 6))
#define GPIOH_PCLK_EN()     (RCC->AHB1ENR |= (1 << 7))
#define GPIOI_PCLK_EN()     (RCC->AHB1ENR |= (1 << 8))

// Clock enable for I2Cx peripherals
#define I2C1_PCLK_EN()      (RCC->APB1ENR |= (1 << 21))
#define I2C2_PCLK_EN()      (RCC->APB1ENR |= (1 << 22))
#define I2C3_PCLK_EN()      (RCC->APB1ENR |= (1 << 23))

// Clock enable for SPIx peripherals
#define SPI1_PCLK_EN()      (RCC->APB2ENR |= (1 << 12))
#define SPI2_PCLK_EN()      (RCC->APB1ENR |= (1 << 14))
#define SPI3_PCLK_EN()      (RCC->APB1ENR |= (1 << 15))
#define SPI4_PCLK_EN()      (RCC->APB2ENR |= (1 << 13))

// Clock enable for USARTx & UARTx peripherals
#define USART1_PCLK_EN()    (RCC->APB2ENR |= (1 << 4))
#define USART2_PCLK_EN()    (RCC->APB1ENR |= (1 << 17))
#define USART3_PCLK_EN()    (RCC->APB1ENR |= (1 << 18))
#define UART4_PCLK_EN()     (RCC->APB1ENR |= (1 << 19))
#define UART5_PCLK_EN()     (RCC->APB1ENR |= (1 << 20))
#define USART6_PCLK_EN()    (RCC->APB2ENR |= (1 << 5))

// Clock enable for SYSCFG peripheral
#define SYSCFG_PCLK_EN()    (RCC->APB2ENR |= (1 << 14))

// Clock enable/disable for TIMx peripheral
#define TIM4_PCLK_EN() 		(RCC->APB1ENR |= (1 << 2))
#define TIM4_PCLK_DI()		(RCC->APB1ENR &= ~(1 << 2))

// Clock disable macros for GPIOx peripherals
#define GPIOA_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 0))
#define GPIOB_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 1))
#define GPIOC_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 2))
#define GPIOD_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 3))
#define GPIOE_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 4))
#define GPIOF_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 5))
#define GPIOG_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 6))
#define GPIOH_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 7))
#define GPIOI_PCLK_DI()		(RCC->AHB1ENR &= ~(1 << 8))

//clock disable macros for SPIx peripherals
#define SPI1_PCLK_DI()      (RCC->APB2ENR &= ~(1 << 12))
#define SPI2_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 14))
#define SPI3_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 15))
#define SPI4_PCLK_DI()      (RCC->APB2ENR &= ~(1 << 13))

// Clock disable for I2Cx peripherals
#define I2C1_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 21))
#define I2C2_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 22))
#define I2C3_PCLK_DI()      (RCC->APB1ENR &= ~(1 << 23))

//clock disable macros for USARTx peripherals
#define USART1_PCLK_DI()    (RCC->APB2ENR &= ~(1 << 4))
#define USART2_PCLK_DI()    (RCC->APB1ENR &= ~(1 << 17))
#define USART3_PCLK_DI()    (RCC->APB1ENR &= ~(1 << 18))
#define UART4_PCLK_DI()     (RCC->APB1ENR &= ~(1 << 19))
#define UART5_PCLK_DI()     (RCC->APB1ENR &= ~(1 << 20))
#define USART6_PCLK_DI()    (RCC->APB2ENR &= ~(1 << 5))

//clock disable macros for  SYSCFG peripheral
#define SYSCFG_PCLK_DI()    (RCC->APB2ENR &= ~(1 << 14))

//macros to reset GPIOx peripherals
#define GPIOA_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 0)); (RCC->AHB1RSTR &= ~(1 << 0)); }while(0)
#define GPIOB_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 1)); (RCC->AHB1RSTR &= ~(1 << 1)); }while(0)
#define GPIOC_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 2)); (RCC->AHB1RSTR &= ~(1 << 2)); }while(0)
#define GPIOD_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 3)); (RCC->AHB1RSTR &= ~(1 << 3)); }while(0)
#define GPIOE_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 4)); (RCC->AHB1RSTR &= ~(1 << 4)); }while(0)
#define GPIOF_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 5)); (RCC->AHB1RSTR &= ~(1 << 5)); }while(0)
#define GPIOG_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 6)); (RCC->AHB1RSTR &= ~(1 << 6)); }while(0)
#define GPIOH_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 7)); (RCC->AHB1RSTR &= ~(1 << 7)); }while(0)
#define GPIOI_REG_RESET()               do{ (RCC->AHB1RSTR |= (1 << 8)); (RCC->AHB1RSTR &= ~(1 << 8)); }while(0)

// Macros to reset SPIx peripherals
#define SPI1_REG_RESET()      			do{ (RCC->APB2RSTR |= (1 << 12)); (RCC->APB2RSTR &= ~(1 << 12)); }while(0)
#define SPI2_REG_RESET()      			do{ (RCC->APB1RSTR |= (1 << 14)); (RCC->APB1RSTR &= ~(1 << 14)); }while(0)
#define SPI3_REG_RESET()      			do{ (RCC->APB1RSTR |= (1 << 15)); (RCC->APB1RSTR &= ~(1 << 15)); }while(0)

// Macros to reset I2Cx periperals
#define I2C1_REG_RESET()      			do{ (RCC->APB1ENR |= (1 << 21)); (RCC->APB1ENR &= ~(1 << 21)); }while(0)
#define I2C2_REG_RESET()      			do{ (RCC->APB1ENR |= (1 << 22)); (RCC->APB1ENR &= ~(1 << 22)); }while(0)
#define I2C3_REG_RESET()      			do{ (RCC->APB1ENR |= (1 << 23)); (RCC->APB1ENR &= ~(1 << 23)); }while(0)


#define GPIOA_BASEADDR_TO_CODE(x)	  (	(x == GPIOA) ? 0:\
										(x == GPIOB) ? 1:\
										(x == GPIOC) ? 2:\
										(x == GPIOD) ? 3:\
										(x == GPIOE) ? 4:\
										(x == GPIOF) ? 5:\
										(x == GPIOG) ? 6:\
										(x == GPIOH) ? 7:\
										(x == GPIOI) ? 8:0 )

//irq numbers
#define IRQ_NO_EXTI0 		6
#define IRQ_NO_EXTI1 		7
#define IRQ_NO_EXTI2 		8
#define IRQ_NO_EXTI3 		9
#define IRQ_NO_EXTI4 		10
#define IRQ_NO_EXTI9_5 		23
#define IRQ_NO_EXTI15_10 	40
#define IRQ_NO_SPI1			35
#define IRQ_NO_SPI2         36
#define IRQ_NO_SPI3         51
#define IRQ_NO_I2C1_EV		31
#define IRQ_NO_I2C1_ER		32


// macros for priority level
#define NVIC_IRQ_PRI0    	0
#define NVIC_IRQ_PRI1    	1
#define NVIC_IRQ_PRI2    	2
#define NVIC_IRQ_PRI3    	3
#define NVIC_IRQ_PRI4    	4
#define NVIC_IRQ_PRI5    	5
#define NVIC_IRQ_PRI6    	6
#define NVIC_IRQ_PRI7    	7
#define NVIC_IRQ_PRI8    	8
#define NVIC_IRQ_PRI9    	9
#define NVIC_IRQ_PRI10   	10
#define NVIC_IRQ_PRI11    	11
#define NVIC_IRQ_PRI12   	12
#define NVIC_IRQ_PRI13    	13
#define NVIC_IRQ_PRI14    	14
#define NVIC_IRQ_PRI15    	15

//generic macros
#define ENABLE				1
#define DISABLE				0
#define SET					ENABLE
#define RESET				DISABLE
#define GPIO_PIN_SET		SET
#define GPIO_PIN_RESET  	RESET
#define FLAG_RESET         	RESET
#define FLAG_SET 			SET

// Bit position definitions SPI_CR1
#define SPI_CR1_CPHA     				 0
#define SPI_CR1_CPOL      				 1
#define SPI_CR1_MSTR     				 2
#define SPI_CR1_BR   					 3
#define SPI_CR1_SPE     				 6
#define SPI_CR1_LSBFIRST   			 	 7
#define SPI_CR1_SSI     				 8
#define SPI_CR1_SSM      				 9
#define SPI_CR1_RXONLY      		 	10
#define SPI_CR1_DFF     			 	11
#define SPI_CR1_CRCNEXT   			 	12
#define SPI_CR1_CRCEN   			 	13
#define SPI_CR1_BIDIOE     			 	14
#define SPI_CR1_BIDIMODE      			15

// Bit position definitions SPI_CR2
#define SPI_CR2_RXDMAEN		 			0
#define SPI_CR2_TXDMAEN				 	1
#define SPI_CR2_SSOE				 	2
#define SPI_CR2_FRF						4
#define SPI_CR2_ERRIE					5
#define SPI_CR2_RXNEIE				 	6
#define SPI_CR2_TXEIE					7

//Bit position definitions SPI_SR
#define SPI_SR_RXNE						0
#define SPI_SR_TXE				 		1
#define SPI_SR_CHSIDE				 	2
#define SPI_SR_UDR					 	3
#define SPI_SR_CRCERR				 	4
#define SPI_SR_MODF					 	5
#define SPI_SR_OVR					 	6
#define SPI_SR_BSY					 	7
#define SPI_SR_FRE					 	8

// Bit position definitions I2C_CR1
#define I2C_CR1_PE						0
#define I2C_CR1_SMBUS					1
#define I2C_CR1_SMTYPE					3
#define I2C_CR1_ENARP					4
#define I2C_CR1_ENPEC					5
#define I2C_CR1_ENGC					6
#define I2C_CR1_NOSTRETCH				7
#define I2C_CR1_START					8
#define I2C_CR1_STOP					9
#define I2C_CR1_ACK						10
#define I2C_CR1_POS						11
#define I2C_CR1_PEC						12
#define I2C_CR1_ALERT					13
#define I2C_CR1_SWRST					15

// Bit position definitions I2C_CR2
#define I2C_CR2_FREQ					0
#define I2C_CR2_ITERREN					8
#define I2C_CR2_ITEVTEN					9
#define I2C_CR2_ITBUFEN					10
#define I2C_CR2_DMAEN					11
#define I2C_CR2_LAST					12


//Bit position definitions I2C_OAR1
#define I2C_OAR1_ADD0    				0
#define I2C_OAR1_ADD1 				 	1
#define I2C_OAR1_ADD8  			 	 	8
#define I2C_OAR1_ADDMODE   			 	15

//Bit position definitions I2C_SR1
#define I2C_SR1_SB 					 	0
#define I2C_SR1_ADDR 				 	1
#define I2C_SR1_BTF 					2
#define I2C_SR1_ADD10 					3
#define I2C_SR1_STOPF 					4
#define I2C_SR1_RXNE 					6
#define I2C_SR1_TXE 					7
#define I2C_SR1_BERR 					8
#define I2C_SR1_ARLO 					9
#define I2C_SR1_AF 					 	10
#define I2C_SR1_OVR 					11
#define I2C_SR1_TIMEOUT 				14

// Bit position definitions I2C_SR2
#define I2C_SR2_MSL						0
#define I2C_SR2_BUSY 					1
#define I2C_SR2_TRA 					2
#define I2C_SR2_GENCALL 				4
#define I2C_SR2_DUALF 					7

//Bit position definitions I2C_CCR
#define I2C_CCR_CCR 					 0
#define I2C_CCR_DUTY 					14
#define I2C_CCR_FS  				 	15

#define TIM1_BASEADDR				0x40010000
#define TIM2_BASEADDR				0x40000000
#define TIM3_BASEADDR				0x40000400
#define TIM4_BASEADDR				0x40000800
#define TIM5_BASEADDR				0x40000C00
#define TIM6_BASEADDR				0x40001000
#define TIM7_BASEADDR				0x40001400
#define TIM8_BASEADDR				0x40010400
#define TIM9_BASEADDR				0x40014000
#define TIM10_BASEADDR				0x40014400
#define TIM11_BASEADDR				0x40014800
#define TIM12_BASEADDR				0x40001800
#define TIM13_BASEADDR				0x40001C00
#define TIM14_BASEADDR				0x40002000

typedef struct
{
	__vo uint32_t CR1;    // 0x00 - Control register 1
	__vo uint32_t CR2;    // 0x04 - Control register 2
	__vo uint32_t SMCR;   // 0x08 - Slave mode control register
	__vo uint32_t DIER;   // 0x0C - Interrupt enable register
	__vo uint32_t SR;     // 0x10 - Status register
	__vo uint32_t EGR;    // 0x14 - Event generation register
	__vo uint32_t CCMR1;  // 0x18 - Capture/compare mode register 1
	__vo uint32_t CCMR2;  // 0x1C - Capture/compare mode register 2
	__vo uint32_t CCER;   // 0x20 - Capture/compare enable register
	__vo uint32_t CNT;    // 0x24 - Counter register
	__vo uint32_t PSC;    // 0x28 - Prescaler register
	__vo uint32_t ARR;    // 0x2C - Auto-reload register
	uint32_t RESERVED; // 0x30
	__vo uint32_t CCR1;   // 0x34 - Capture/compare register 1
	__vo uint32_t CCR2;   // 0x38 - Capture/compare register 2
	__vo uint32_t CCR3;   // 0x3C - Capture/compare register 3
	__vo uint32_t CCR4;   // 0x40 - Capture/compare register 4
} TIM_RegDef_t;

#define TIM1                ((TIM_RegDef_t*)TIM1_BASEADDR)
#define TIM2                ((TIM_RegDef_t*)TIM2_BASEADDR)
#define TIM3                ((TIM_RegDef_t*)TIM3_BASEADDR)
#define TIM4                ((TIM_RegDef_t*)TIM4_BASEADDR)
#define TIM5                ((TIM_RegDef_t*)TIM5_BASEADDR)
#define TIM6                ((TIM_RegDef_t*)TIM6_BASEADDR)
#define TIM7                ((TIM_RegDef_t*)TIM7_BASEADDR)
#define TIM8                ((TIM_RegDef_t*)TIM8_BASEADDR)
#define TIM9                ((TIM_RegDef_t*)TIM9_BASEADDR)
#define TIM10                ((TIM_RegDef_t*)TIM10_BASEADDR)
#define TIM11                ((TIM_RegDef_t*)TIM11_BASEADDR)
#define TIM12                ((TIM_RegDef_t*)TIM12_BASEADDR)
#define TIM13                ((TIM_RegDef_t*)TIM13_BASEADDR)
#define TIM14                ((TIM_RegDef_t*)TIM14_BASEADDR)

#include "stm32f407_i2c_driver.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_spi_driver.h"

#endif /* INC_STM32F407_H_ */
