/*
 * LIS2DW12.h
 *
 *  Created on: Jun 11, 2023
 *      Author: Tom Evison
 */

#ifndef INC_LIS2DW12_H_
#define INC_LIS2DW12_H_

#include "stm32f4xx_hal.h" // cubeIDE HAL is used

// defines
#define WHO_AM_I 0x44 //lis2dw12/8.3
#define LISaddr 0b0011001 // SA0 Pulled up internally > LSB of device addr =1
#define i2cTout 1000 // i2c timeout

// registers
#define LIS2DW12_REG_OUT_T_L   0x0D
#define LIS2DW12_REG_OUT_T_H   0x0E
#define LIS2DW12_REG_WHOAMI    	0x0F
#define LIS2DW12_REG_CTRL1  0x20
#define LIS2DW12_REG_CTRL2 0x21
#define LIS2DW12_REG_CTRL3 0x22
#define LIS2DW12_REG_CTRL4_INT1_PAD_CTRL  0x23
#define LIS2DW12_REG_CTRL4_INT2_PAD_CTRL 0x24
#define LIS2DW12_REG_CTRL6 0x25
#define LIS2DW12_REG_OUT_T 0x26
#define LIS2DW12_REG_STATUS 0x27
#define LIS2DW12_REG_OUT_X_L 0x28
#define LIS2DW12_REG_OUT_X_H 0x29
#define LIS2DW12_REG_OUT_Y_L 0x2A
#define LIS2DW12_REG_OUT_Y_H 0x2B
#define LIS2DW12_REG_OUT_Z_L 0x2C
#define LIS2DW12_REG_OUT_Z_H 0x2D
#define LIS2DW12_REG_FIFO_CTRL 0x2E

// CTRL 1
#define ODR      0b0101
#define MODE       0b10
#define LP_MODE    0b01

// CTRL 2
#define BOOT        0b0 // 0: disabled; 1: enabled
#define SOFT_RESET  0b0 // 0: disabled; 1: enabled
#define ZERO        0b0 // This bit must be set to ‘0’ for the correct operation of the device
#define CS_PU_DISC  0b0 // 0: pull-up connected to CS pin 1: pull-up disconnected to CS pin
#define BDU         0b1 // 0: continuous update; 1: output registers not updated until MSB and LSB read
#define IF_ADD_INC  0b1 // 0: disabled; 1: enabled
#define I2C_DISABLE 0b0 // 0: SPI and I²C interfaces enabled; 1: I²C mode disabled
#define SIM         0b0 // 0: 4-wire interface; 1: 3-wire interface

// CTRL 3
#define ST           0b00 // 00: Self-test disabled; Other: see Table 35. Self-test mode selection
#define PP_OD        0b00 // 0: push-pull; 1: open-drain
#define LIR          0b00 // 0: interrupt request not latched; 1: interrupt request latched)
#define H_LACTIVE    0b00 // 0: active high; 1: active low
#define SLP_MODE_SEL 0b01 // 0: enabled with external trigger on INT2, 1: enabled by I²C/SPI writing SLP_MODE_1 to 1
#define SLP_MODE_1   0b00 // SLP_MODE_SEL = '1' and this bit is set to '1' logic, single data conversion on demand mode starts.

// CTRL 6
#define BW_FILT   0b00 // Bandwidth selection (see Table 41. Digital filtering cutoff selection)
#define FS        0b00 // Full-scale selection (see Table 42. Full-scale selection)
#define FDS       0b00 // 0: low-pass filter path selected, 1: high-pass filter path selected
#define LOW_NOISE 0b01 // 0: disabled; 1: enabled

typedef struct LIS2{
	I2C_HandleTypeDef* i2cHandl;
	int8_t temp_8bit;
	double temp_12bit;
	double accX;
	double accY;
	double accZ;


} LIS2 ;

HAL_StatusTypeDef LIS2_Init(LIS2 *dev, I2C_HandleTypeDef *hi2c1);
HAL_StatusTypeDef LIS2_Refresh(LIS2 *dev);
uint8_t LIS2_getStatus(LIS2 *dev);
uint8_t LIS2_reset(LIS2 *dev);

#endif /* INC_LIS2DW12_H_ */
