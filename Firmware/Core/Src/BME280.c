#include "BME280.h"

	BME280comp comp; // struct of compensation values
	int32_t t_fine;  // carries a fine resolution temperature value

HAL_StatusTypeDef BME280_Init(BME280 *dev, SPI_HandleTypeDef *hspi1, GPIO_TypeDef *CSB_Port, uint16_t CSB_Pin ){

	dev->spiHandl = hspi1;
	dev->CSB_port = CSB_Port;
	dev->CSB_pin = CSB_Pin;

	uint8_t status = 0;

	// check the device ID and abort is invalid
	uint8_t deviceId;
	BME280_readReg(dev, BME280_REG_DEVID, &deviceId);
	if(deviceId != BME280_DEVID){return HAL_ERROR;}

	// read and process all compensation registers
	BME280_setCompensation(dev);

	// set humidity over-sampling to x1
	BME280_writeReg(dev, BME280_REG_CTRLHUM, 1);

	// set temperature over-sampling to x1
	uint8_t ctrl_meas;
	BME280_readReg(dev, BME280_REG_CTRLMES, &ctrl_meas);
	ctrl_meas |= 1 << 5; // set temperature over-sampling to x1
	ctrl_meas |= 1 << 2; // set pressure over-sampling to x1
	BME280_writeReg(dev, BME280_REG_CTRLMES, ctrl_meas);

	// Set standby timer ( t_sb<2:0> = config<7:5> )to 011 = 250ms
	// note this will only be used if device is in mode 'normal'
	uint8_t config_reg;
	BME280_readReg(dev, BME280_REG_CONFIG, &config_reg);
	config_reg &= ~(1 << 7); // t_sb:2
	config_reg |= 1 << 6;	 // t_sb:1
	config_reg |= 1 << 5;    // t_sb:0

	return HAL_OK;
}

// set device mode (bst-bme280-ds002/3.3.1)
BME280_MODE BME280_setMode(BME280 *dev, BME280_MODE mode){

	uint8_t ctrl_meas;

	switch (mode){
		case BME280_SLEEP:
	      // statements
	      break;

	    case BME280_NORMAL:
	    	// set to normal mode: Mode[1:0] = 11
	    	BME280_readReg(dev, BME280_REG_CTRLMES, &ctrl_meas);
	    	ctrl_meas |= BME280_NORMAL;
	    	BME280_writeReg(dev, BME280_REG_CTRLMES, ctrl_meas);
	      break;
	    case BME280_FORCED:
	    	// then transition to force mode: Mode[1:0] = 01
			BME280_readReg(dev, BME280_REG_CTRLMES, &ctrl_meas);
			ctrl_meas &= ~(1 << 1);
			ctrl_meas |= 1 << 0;
			BME280_writeReg(dev, BME280_REG_CTRLMES, ctrl_meas);
			BME280_readReg(dev, BME280_REG_CTRLMES, &ctrl_meas);

	      break;
	    default:
	    	return HAL_ERROR;
	    break;
	}
	return HAL_OK;
}

HAL_StatusTypeDef BME280_updateValues(BME280 *dev){
	// start conversion
	uint8_t status =0;
	BME280_setMode(dev, BME280_NORMAL);
	BME280_setMode(dev, BME280_FORCED);
	BME280_readReg(dev, BME280_REG_STATUS, &status);

	BME280_getTemp(dev); // (C)
	BME280_getHumd(dev); // (%)
	BME280_getPres(dev); // {Pa)
	return HAL_OK;
}

HAL_StatusTypeDef BME280_getTemp(BME280 *dev){

	// read register
	uint8_t temp[3] = {0};
	BME280_readRegs(dev, BME280_REG_TEMPMSB, &temp[0], 3);
	int32_t raw_temp = 0;
	raw_temp |= temp[2] >> 4; /// XLSB
	raw_temp |= temp[1] << 4;  // LSB
	raw_temp |= temp[0] << 12; // MSB

	dev->temp = BME280_compensate_Temp(raw_temp);
	return HAL_OK;
}

HAL_StatusTypeDef BME280_getHumd(BME280 *dev){

	uint8_t temp[2] = {0};
	BME280_readRegs(dev, BME280_REG_HUMMSB, &temp[0],2);
	int32_t raw_humd = temp[1]; // LSB
	raw_humd |= temp[0] << 8;   // MSB

	dev->humd = BME280_compensate_Humd(raw_humd);
	return HAL_OK;
}

HAL_StatusTypeDef BME280_getPres(BME280 *dev){

	uint8_t temp_pres[3] = {0};
	BME280_readRegs(dev, BME280_REG_PRESMSB, &temp_pres[0],3);
	int32_t raw_pres = 0;
	raw_pres |= temp_pres[2] >> 4; // XLSB
	raw_pres |= temp_pres[1] << 4;        // LSB
	raw_pres |= temp_pres[0] << 12;       // MSB

	dev->pres = BME280_compensate_Pres(raw_pres)/100;
	return HAL_OK;
}


/*
 * Reading is done by lowering CSB and first sending one control byte. The control bytes consist of the
 * SPI register address (= full register address without bit 7) and the read command (bit 7 = RW = ‘1’).
 * After writing the control byte, data is sent out of the SDO pin (SDI in 3-wire mode); the register address
 * is automatically incremented. The SPI read protocol is depicted in Figure 13
 */
HAL_StatusTypeDef BME280_readReg(BME280 *dev, uint8_t reg, uint8_t *data){

	HAL_StatusTypeDef status;

	reg |= 0x80;  // read operation - bit 7 always 1.
	BME280_CSBLow(dev);
	HAL_SPI_Transmit (dev->spiHandl, &reg, 1, TIMEOUT_ms);  // send address
	status = HAL_SPI_Receive (dev->spiHandl, data, 1, TIMEOUT_ms);  // receive 6 bytes data
	BME280_CSBHigh(dev);
	return status;
}

HAL_StatusTypeDef BME280_readRegs(BME280 *dev, uint8_t reg, uint8_t *data, uint8_t length){

	HAL_StatusTypeDef status;

	reg |= 0x80;  // read operation - bit 7 always 1.
	BME280_CSBLow(dev);
	HAL_SPI_Transmit (dev->spiHandl, &reg, 1, TIMEOUT_ms);  // send address
	status = HAL_SPI_Receive (dev->spiHandl, data, length, TIMEOUT_ms);  // receive 6 bytes data
	BME280_CSBHigh(dev);
	return status;

}
/*
 * Writing is done by lowering CSB and sending pairs control bytes and register data. The control bytes
 * consist of the SPI register address (= full register address without bit 7) and the write command (bit7 =
 * RW = ‘0’). Several pairs can be written without raising CSB. The transaction is ended by a raising
 * CSB. The SPI write protocol is depicted in Figure 12
 */
HAL_StatusTypeDef BME280_writeReg(BME280 *dev, uint8_t reg, uint8_t data){

	HAL_StatusTypeDef status;
	uint8_t write_cmd[2] = {0};

	// built write command
	write_cmd[0] = reg & ~(1 << 7); // clear MSB for 'write' operations (bst-bme280-ds002/6.3.1)
	write_cmd[1] = data;

	BME280_CSBLow(dev);
	status = HAL_SPI_Transmit (dev->spiHandl, (uint8_t*)write_cmd, 2, TIMEOUT_ms);  // send address
	BME280_CSBHigh(dev);
	return status;
}

HAL_StatusTypeDef BME280_CSBHigh(BME280 *dev){
	HAL_GPIO_WritePin(dev->CSB_port, dev->CSB_pin, SET); // end transaction by pulling CSB high
	return HAL_OK;
}

HAL_StatusTypeDef BME280_CSBLow(BME280 *dev){
	HAL_GPIO_WritePin(dev->CSB_port, dev->CSB_pin, RESET); // chip select is active low
	return HAL_OK;
}

// compensation formula
HAL_StatusTypeDef BME280_setCompensation(BME280 *dev){
	// read compensation values
	uint8_t raw_comp[20] = {0};
	// temp compensation
	BME280_readRegs(dev, 0x88, &raw_comp[0], 6);
	comp.T1 = raw_comp[0];
	comp.T1 |= raw_comp[1] << 8;
	comp.T2 = raw_comp[2];
	comp.T2 |= raw_comp[3] << 8;
	comp.T3 = raw_comp[4];
	comp.T3 |= raw_comp[5] << 8;

	// humidity compensation
	BME280_readRegs(dev, 0xA1, &comp.H1, 1);
	BME280_readRegs(dev, 0xE1, &raw_comp[0], 7);
	comp.H2 = raw_comp[0];
	comp.H2 |= raw_comp[1] << 8;
	comp.H3 = raw_comp[2];
	comp.H4 = raw_comp[3] << 4;
	comp.H4 |= raw_comp[4];
	comp.H5 = raw_comp[5];
	comp.H5 = raw_comp[6] << 4;

	// pressure compensation
	BME280_readRegs(dev, 0x8E, &raw_comp[0], 18);
	comp.P1  = raw_comp[0];
	comp.P1 |= raw_comp[1] << 8;
	comp.P2  = raw_comp[2];
	comp.P2 |= raw_comp[3] << 8;
	comp.P3  = raw_comp[4];
	comp.P3 |= raw_comp[5] << 8;
	comp.P4  = raw_comp[6];
	comp.P4 |= raw_comp[7] << 8;
	comp.P5  = raw_comp[8];
	comp.P5 |= raw_comp[9] << 8;
	comp.P6  = raw_comp[10];
	comp.P6 |= raw_comp[11] << 8;
	comp.P7  = raw_comp[12];
	comp.P7 |= raw_comp[13] << 8;
	comp.P8  = raw_comp[14];
	comp.P8 |= raw_comp[15] << 8;
	comp.P9  = raw_comp[16];
	comp.P9 |= raw_comp[17] << 8;
	return HAL_OK;
}

double BME280_compensate_Temp(int32_t adc_T){
	double var1, var2, T;
	var1=(((double)adc_T)/16384.0-((double)comp.T1)/1024.0)*((double)comp.T2);
	var2=((((double)adc_T)/131072.0-((double)comp.T1)/8192.0)*(((double)adc_T)/131072.0-((double)comp.T1)/8192.0))*((double)comp.T3);
	t_fine=(int32_t)(var1+var2);
	T=(var1+var2)/5120.0;
	return T;
}

double BME280_compensate_Humd(int32_t adc_H){
	double var_H;
	var_H = (((double)t_fine) - 76800.0);
	var_H = (adc_H - (((double)comp.H4) * 64.0 + ((double)comp.H5) / 16384.0 *
	var_H)) * (((double)comp.H2) / 65536.0 * (1.0 + ((double)comp.H6) /
			67108864.0 * var_H *
			(1.0 + ((double)comp.H3) / 67108864.0 * var_H)));
	var_H = var_H * (1.0 - ((double)comp.H1) * var_H / 524288.0);
	if (var_H > 100.0)
		var_H = 100.0;
	else if (var_H < 0.0)
		var_H = 0.0;
	return var_H;
}

// Returns pressure in Pa as double. Output value of “96386.2” equals 96386.2 Pa = 963.862 hPa
double BME280_compensate_Pres(int32_t adc_P){
	double var1, var2, p;
	var1 = ((double)t_fine/2.0) - 64000.0;
	var2 = var1 * var1 * ((double)comp.P6) / 32768.0;
	var2 = var2 + var1 * ((double)comp.P5) * 2.0;
	var2 = (var2/4.0)+(((double)comp.P4) * 65536.0);
	var1 = (((double)comp.P3) * var1 * var1 / 524288.0 + ((double)comp.P2) * var1) / 524288.0;
	var1 = (1.0 + var1 / 32768.0)*((double)comp.P1);
	if (var1 == 0.0){
		return 0; // avoid exception caused by division by zero
	}
	p = 1048576.0 - (double)adc_P;
	p = (p - (var2 / 4096.0)) * 6250.0 / var1;
	var1 = ((double)comp.P9) * p * p / 2147483648.0;
	var2 = p * ((double)comp.P8) / 32768.0;
	p = p + (var1 + var2 + ((double)comp.P7)) / 16.0;
	return p;
}
