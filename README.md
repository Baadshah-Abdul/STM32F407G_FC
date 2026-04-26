# STM32F407G Flight Controller

A bare-metal quadcopter flight controller written for the STM32F407G Discovery board.
All peripheral drivers are written from scratch using direct register access with no HAL.
The project is built for learning purposes and indoor flight testing on an F450 frame.

---

## Hardware

| Component | Details |
|---|---|
| Flight controller | STM32F407G Discovery |
| IMU | MPU6500 (SPI) |
| ESCs | 4x standard PWM ESCs |
| Frame | F450 quadcopter |
| Radio receiver | RadioMaster XR1 Nano Dual (ELRS 2.4GHz) |
| Transmitter | RadioMaster Pocket |
| Power | 3S/4S LiPo with 5V BEC to board VDD |

---

## Repository Structure

```
STM32F407G_FC/
|
|-- Src/
|   |-- main.c                      Flight controller entry point
|   |-- mpu6500.c                   IMU driver and sensor fusion
|   |-- pid.c                       PID controller
|   |-- pwm.c                       ESC PWM output
|
|-- drivers/
|   |-- stm32f407_gpio_driver.c/h   GPIO peripheral driver
|   |-- stm32f407_spi_driver.c/h    SPI peripheral driver
|   |-- stm32f407_i2c_driver.c/h    I2C peripheral driver (in progress)
|   |-- stm32f407_timer.c/h         SysTick and general purpose timer driver
|   |-- stm32f407.h                 STM32F407 register definitions and base addresses
```

---

## Features

- Bare-metal register-level drivers for GPIO, SPI, I2C, and timers
- MPU6500 IMU over SPI with burst read of accel and gyro data
- Sensor calibration on startup averaging 1000 samples
- Low pass filter on accel and gyro data
- Complementary filter for roll and pitch angle estimation
- Initial attitude estimation from accelerometer at boot regardless of surface angle
- PID control for roll, pitch, and yaw rate with integral windup limiting and derivative spike protection
- PWM output at 50Hz on TIM4 channels (PD12-PD15) for 4 ESCs
- Standard X-frame motor mixing with full roll, pitch, and yaw correction
- 30 second safety disarm with integral reset

---

## Motor Layout

```
        FRONT
   M1 (CW)    M2 (CCW)
      \          /
       \        /
   M3 (CCW)  M4 (CW)
        BACK
```

PWM pin mapping:

| Motor | Pin | TIM4 Channel |
|---|---|---|
| M1 Front Left | PD12 | CH1 |
| M2 Front Right | PD13 | CH2 |
| M3 Back Left | PD14 | CH3 |
| M4 Back Right | PD15 | CH4 |

---

## Pin Assignments

| Peripheral | Pins | Notes |
|---|---|---|
| SPI1 (MPU6500) | PA5 SCK, PA6 MISO, PA7 MOSI | |
| MPU6500 CS | PA4 | GPIO output |
| I2C1 (BMP280, future) | PB6 SCL, PB7 SDA | 4.7k pull-ups required |
| TIM4 PWM | PD12-PD15 | 50Hz, 1000-2000us range |
| USART2 (CRSF, future) | PA2 TX, PA3 RX | 420000 baud, inverted |

---

## Clock Configuration

The board runs on the internal HSI oscillator at 16MHz. No PLL is configured.
SysTick is initialised for 1ms intervals. The main control loop runs at 200Hz (every 5ms).

---

## PID

Three independent PID controllers run in the main loop.

| Axis | Input | Control type |
|---|---|---|
| Roll | sensor roll angle (degrees) | Angle |
| Pitch | sensor pitch angle (degrees) | Angle |
| Yaw | gyro Z rate (degrees/second) | Rate |

Yaw uses rate control because the accelerometer cannot measure rotation around the gravity axis.
A magnetometer would be required for absolute yaw angle estimation.

---

## Toolchain

- IDE: STM32CubeIDE
- Compiler: arm-none-eabi-gcc
- Debug: ST-Link with CubeMonitor for live variable monitoring
- No HAL, no CMSIS DSP, no RTOS

---

## Work in Progress

- BMP280 barometer over I2C (altitude hold, future)
- USART driver (in course)
- CRSF protocol parser for RadioMaster XR1 Nano receiver
- CRSF channel mapping to throttle, roll, pitch, yaw setpoints and arm switch
- Standalone printf via USART2 retargeting

---

## Learning Reference

Peripheral drivers follow the structure taught in the Udemy course
Mastering Microcontroller with Peripheral Driver Development by Kiran Nayak.
Timer and sensor fusion code written independently.
