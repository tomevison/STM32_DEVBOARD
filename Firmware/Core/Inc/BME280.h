/*
 * Bosch BME280 SPI Driver
 * Author:  Tom Evison
 * Created: 11/6/2023
 *
 */

#ifndef BME280_SPI_DRIVER
#define BME280_SPI_DRIVER

#include "stm32f4xx_hal.h" // cubeIDE HAL is used

// defines
#define BME280_DEVID 0x60 // (bst-bme280-ds002/5.4.1) The “id” register contains the chip identification number chip_id[7:0], which is 0x60. This number can be read as soon as the device finished the power-on-reset
#define TIMEOUT_ms 1000   // set timeout for SPI reads

// registers
#define BME280_REG_HUMLSB   0xFE
#define BME280_REG_HUMMSB   0xFD
#define BME280_REG_TEMPXLSB 0xFC //temp_xlsb<7:4>
#define BME280_REG_TEMPLSB  0xFB
#define BME280_REG_TEMPMSB  0xFA
#define BME280_REG_PRESXLSB 0xF9 //pres_xlsb<7:4>
#define BME280_REG_PRESLSB  0xF8
#define BME280_REG_PRESMSB  0xF7
#define BME280_REG_CONFIG   0xF5 // t_sb<7:5> filter<4:2> spi3w_en<0>
#define BME280_REG_CTRLMES  0xF4 // osrs_t<7:5> osrs_p<4:2> mode<1:0>
#define BME280_REG_STATUS   0xF3
#define BME280_REG_CTRLHUM  0xF2
#define BME280_REG_RESET    0xE0
#define BME280_REG_DEVID    0xD0

typedef struct BME280{
	GPIO_TypeDef* CSB_port;
	uint16_t CSB_pin;
	SPI_HandleTypeDef* spiHandl;
	double humd;
	double temp;
	double pres;
} BME280 ;

typedef enum { // (bst-bme280-ds002/Table 25)
  BME280_SLEEP  = 0b00,
  BME280_NORMAL = 0b11,
  BME280_FORCED = 0b01,
} BME280_MODE;

typedef struct BME280comp{ // (bst-bme280-ds002/Table 16)
	uint16_t T1; // unsigned short
	int16_t  T2; // signed short
	int16_t  T3; // signed short
	uint8_t  H1; // unsigned char
	int16_t  H2; // signed short
	uint8_t  H3; // unsigned char
	int16_t  H4; // signed short
	int16_t  H5; // signed short
	int8_t   H6; // signed char
	uint16_t P1;
	int16_t  P2;
	int16_t  P3;
	int16_t  P4;
	int16_t  P5;
	int16_t  P6;
	int16_t  P7;
	int16_t  P8;
	int16_t  P9;
} BME280comp;

extern BME280comp comp; // struct of compensation values
extern int32_t t_fine;  // holds a fine resolution temperature value used for pres / humd compensation

// function defs
HAL_StatusTypeDef BME280_Init(BME280 *dev, SPI_HandleTypeDef *hspi1, GPIO_TypeDef *CSB_Port, uint16_t CSB_Pin );
BME280_MODE BME280_setMode(BME280 *dev, BME280_MODE mode);
HAL_StatusTypeDef BME280_updateValues(BME280 *dev);
HAL_StatusTypeDef BME280_getHumd(BME280 *dev);
HAL_StatusTypeDef BME280_getTemp(BME280 *dev);
HAL_StatusTypeDef BME280_getPres(BME280 *dev);

// low level functions
HAL_StatusTypeDef BME280_readReg(BME280 *dev, uint8_t reg, uint8_t *data);
HAL_StatusTypeDef BME280_readRegs(BME280 *dev, uint8_t reg, uint8_t *data, uint8_t length);
HAL_StatusTypeDef BME280_writeReg(BME280 *dev, uint8_t reg, uint8_t data);
HAL_StatusTypeDef BME280_CSBHigh(BME280 *dev);
HAL_StatusTypeDef BME280_CSBLow(BME280 *dev);

// compensation
HAL_StatusTypeDef BME280_setCompensation(BME280 *dev);
double BME280_compensate_Temp(int32_t adc_T);
double BME280_compensate_Humd(int32_t adc_H);
double BME280_compensate_Pres(int32_t adc_P);

#endif
