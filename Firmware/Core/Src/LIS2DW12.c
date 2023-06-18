/*
 * LIS2DW12.c
 *
 *  Created on: Jun 11, 2023
 *      Author: Tom Evison
 */
#include "LIS2DW12.h"

uint8_t buf[20] = {0};
HAL_StatusTypeDef ret;

HAL_StatusTypeDef LIS2_Init(LIS2 *dev, I2C_HandleTypeDef *hi2c1){

	dev->i2cHandl = hi2c1;

	// check device ID
	buf[0] = LIS2DW12_REG_WHOAMI;
	ret = HAL_I2C_Mem_Read(hi2c1, (LISaddr<<1), LIS2DW12_REG_WHOAMI, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	if (( ret != HAL_OK ) || buf[0] != WHO_AM_I) {return HAL_ERROR; }

	LIS2_reset(dev);

	// Control Register 1
	// LP_MODE[1:0]
	buf[0] = (ODR<<4) | (MODE<<2) | LP_MODE;
	ret = HAL_I2C_Mem_Write(hi2c1, (LISaddr<<1), LIS2DW12_REG_CTRL1, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	// Control Register 2
	buf[0] = (BOOT<<7) |(SOFT_RESET<<6)|(ZERO<<5)|(CS_PU_DISC<<4) |(BDU<<3) |(IF_ADD_INC<<2) | (I2C_DISABLE<<1) | SIM;
	ret = HAL_I2C_Mem_Write(hi2c1, (LISaddr<<1), LIS2DW12_REG_CTRL2, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	// Control Register 3
	buf[0] = (ST<<6)|(PP_OD<<5)|(LIR<<4) |(H_LACTIVE<<3) | (SLP_MODE_SEL<<1) | SLP_MODE_1;
	ret = HAL_I2C_Mem_Write(hi2c1, (LISaddr<<1), LIS2DW12_REG_CTRL3, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	// Control Register 6
	buf[0] = (BW_FILT<<6)|(FS<<4) |(FDS<<3) | (LOW_NOISE<<2);
	ret = HAL_I2C_Mem_Write(hi2c1, (LISaddr<<1), LIS2DW12_REG_CTRL6, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	// read all control registers
	ret = HAL_I2C_Mem_Read(hi2c1, (LISaddr<<1), LIS2DW12_REG_CTRL1, I2C_MEMADD_SIZE_8BIT, &buf[0], 6, i2cTout);

	// read fifo
	ret = HAL_I2C_Mem_Read(hi2c1, (LISaddr<<1), LIS2DW12_REG_FIFO_CTRL, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	buf[0] |= 1 <<6; // Fmode[2:0]=0b110 fifo mode continuous
	buf[0] |= 1 <<7;
	ret = HAL_I2C_Mem_Write(hi2c1, (LISaddr<<1), LIS2DW12_REG_FIFO_CTRL, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	return HAL_OK;
}

HAL_StatusTypeDef LIS2_Refresh(LIS2 *dev){

	// read status
	ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_CTRL3, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	buf[0] |= 1 << 0; // SLP_MODE_1
	ret = HAL_I2C_Mem_Write(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_CTRL3, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);

	while(buf[0] & (1 << 0)){
		ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_CTRL3, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
		HAL_Delay(2);
	}; // wait for SLP_MODE_1 to == 0

	// read temperature
	ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_OUT_T, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	dev->temp_8bit = (buf[0] + 25);

	// read temperature 12bit res
	ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_OUT_T_L, I2C_MEMADD_SIZE_8BIT, &buf[0], 2, i2cTout);
	dev->temp_12bit = (buf[0] | buf[1]<<8) + 25;

	// read orientation z
	ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_OUT_X_L, I2C_MEMADD_SIZE_8BIT, &buf[0], 6, i2cTout);
	dev->accX = ((int16_t)(buf[0] | buf[1]<<8) /4.0)*0.244; // g
	dev->accY = ((int16_t)(buf[2] | buf[3]<<8) /4.0)*0.244; // g
	dev->accZ = ((int16_t)(buf[4] | buf[5]<<8) /4.0)*0.244; // g


	return HAL_OK;
}

uint8_t LIS2_getStatus(LIS2 *dev){
	return 0xFF;
}

HAL_StatusTypeDef LIS2_reset(LIS2 *dev){
	ret = HAL_I2C_Mem_Read(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_CTRL2, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	buf[0] |= 0x40;
	ret = HAL_I2C_Mem_Write(dev->i2cHandl, (LISaddr<<1), LIS2DW12_REG_CTRL2, I2C_MEMADD_SIZE_8BIT, &buf[0], 1, i2cTout);
	HAL_Delay(20);
	return HAL_OK;

}


