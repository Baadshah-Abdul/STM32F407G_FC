/**
 * bmp280.h
 * BMP280 Digital Pressure Sensor Driver for STM32F407
 *
 * Created on: May 2026
 * Author: ACER1
 *
 * Hardware Connections:
 * BMP280 SDA    -> PB11 (I2C2_SDA)
 * BMP280 SCL    -> PB10 (I2C2_SCL)
 * BMP280 CSB    -> 3.3V   (Required for I2C mode)
 * BMP280 SDO    -> GND    (I2C address 0x76)
 * BMP280 VDD    -> 3.3V
 * BMP280 VDDIO  -> 3.3V
 * BMP280 GND    -> GND
 */

#ifndef INC_BMP280_H_
#define INC_BMP280_H_

#include <stdint.h>
#include "stm32f407.h"
#include "stm32f407_gpio_driver.h"
#include "stm32f407_i2c_driver.h"

/* ==========================================================================
   BMP280 Register Addresses
   ========================================================================== */
#define BMP280_ID               0xD0
#define BMP280_RESET            0xE0
#define BMP280_STATUS           0xF3
#define BMP280_CTRL_MEAS        0xF4
#define BMP280_CONFIG           0xF5
#define BMP280_PRESS_MSB        0xF7
#define BMP280_PRESS_LSB        0xF8
#define BMP280_PRESS_XLSB       0xF9
#define BMP280_TEMP_MSB         0xFA
#define BMP280_TEMP_LSB         0xFB
#define BMP280_TEMP_XLSB        0xFC
#define BMP280_CALIB_START      0x88

/* ==========================================================================
   BMP280 Commands and Values
   ========================================================================== */
#define BMP280_RESET_CMD            0xB6
#define BMP280_CHIP_ID              0x58

/* Forced mode: single measurement then sleep
   osrs_t = 001 (temperature x1)
   osrs_p = 011 (pressure x4)
   mode   = 01  (forced)
   = 0x25 */
#define BMP280_CTRL_MEAS_FORCED     0x25

/* config: no filter, no standby */
#define BMP280_CONFIG_VALUE         0x00

/* Measurement delay (pressure x4 + temp x1 = ~11.5ms max, use 15ms margin) */
#define BMP280_MEAS_DELAY_MS        15

/* I2C Address (7-bit) */
#define BMP280_I2C_ADDR             0x76

/* ==========================================================================
   BMP280 Calibration Data Structure
   ========================================================================== */
typedef struct
{
    uint16_t dig_T1;
    int16_t  dig_T2;
    int16_t  dig_T3;
    uint16_t dig_P1;
    int16_t  dig_P2;
    int16_t  dig_P3;
    int16_t  dig_P4;
    int16_t  dig_P5;
    int16_t  dig_P6;
    int16_t  dig_P7;
    int16_t  dig_P8;
    int16_t  dig_P9;
} BMP280_CalibData_t;

/* ==========================================================================
   BMP280 Final Data Structure
   ========================================================================== */
typedef struct
{
    uint32_t pressure_pa;           /* Compensated pressure in Pascals */
    int32_t  altitude_cm;           /* Altitude in centimeters */
    uint32_t ground_pressure_pa;    /* Reference pressure at takeoff */

    /* Raw ADC values (internal use) */
    int32_t  adc_P;
    int32_t  adc_T;
    int32_t  t_fine;

    /* Calibration data (internal use) */
    BMP280_CalibData_t CalibData;

} BMP280_FinalValue_t;

/* ==========================================================================
   Function Prototypes
   ========================================================================== */

/* I2C Peripheral Initializations */
void I2C1_GPIO_Config(void);            /* Setup PB6, PB7 for I2C1 */
void I2C1_Config(void);                 /* Setup I2C1 peripheral */

/* BMP280 Specific */
void BMP280_Read_RawData(void);
void BMP280_CompensateData(void);
BMP280_FinalValue_t* BMP280_GetData(void);
void BMP280_CalibrateGroundPressure(void);

#endif /* INC_BMP280_H_ */
