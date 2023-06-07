#include "main.h"

struct BME280{
	GPIO_TypeDef* port;
	uint16_t pin;
	SPI_HandleTypeDef* spi;
};

struct BME280 bm;

uint32_t BME280_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, SPI_HandleTypeDef* hspi1){

	bm.port = GPIOx;
	bm.pin = GPIO_Pin;
	bm.spi = hspi1;

	uint8_t address = 0;
	address |= 0x80;  // read operation
	address |= 0xD0;  // "id" register
	uint8_t data_rec[1] = {0};

	// read temp data from BME280 via SPI
	HAL_GPIO_WritePin(bm.port, bm.pin, RESET); // chip select is active low
	HAL_SPI_Transmit (bm.spi, &address, 1, 100);  // send address
	HAL_SPI_Receive (bm.spi, data_rec, 1, 100);  // receive 6 bytes data
	HAL_GPIO_WritePin(bm.port, bm.pin, SET); // end transaction by pulling CSB high

	// check sensor is valid
	if(data_rec[0]==0x60){
		return 1;
	}else{
		return -1;
	}
}

BME280_setMode(){

	// first we must get mode.
	uint8_t address = 0;
	address |= 0x80;  // read operation
	address |= 0xF4;  // "id" register
	uint8_t data_rec[1] = {0};
	uint8_t data_snd[2] = {0};

	// read mode data from BME280 via SPI
	HAL_GPIO_WritePin(bm.port, bm.pin, RESET); // chip select is active low
	HAL_SPI_Transmit (bm.spi, &address, 1, 100);  // send address
	HAL_SPI_Receive (bm.spi, data_rec, 1, 100);  // receive 6 bytes data
	HAL_GPIO_WritePin(bm.port, bm.pin, SET); // end transaction by pulling CSB high

	data_snd[0] = address & ~(1 << 7);  // write address
	data_snd[1] = data_rec[0] | 1 << 0; // mode[1:0] 10 - normal mode

	// read mode data from BME280 via SPI
	HAL_GPIO_WritePin(bm.port, bm.pin, RESET); // chip select is active low
	HAL_SPI_Transmit (bm.spi, data_snd, 2, 1000);  // send address
	HAL_GPIO_WritePin(bm.port, bm.pin, SET); // end transaction by pulling CSB high

	return 1;
}

BME280_setTempOS(){

}
BME280_setPresOS(){

}
BME280_setHumdOS(){

	uint8_t data_snd[2] = {0};
	uint8_t data_rec[2] = {0};
	data_snd[0] = 0xF2 & ~(1 << 7);  // write address
	data_snd[1] = 0b0001;  // oversampling x1


	// read mode data from BME280 via SPI
	HAL_GPIO_WritePin(bm.port, bm.pin, RESET); // chip select is active low
	HAL_SPI_Transmit (bm.spi, data_snd, 2, 1000);  // send address
	HAL_GPIO_WritePin(bm.port, bm.pin, SET); // end transaction by pulling CSB high

}

uint32_t BME280_getHumd(){
	uint8_t address = 0;
	address |= 0x80;  // read operation
	address |= 0xFE;  // "humidity" register
	uint8_t data_rec[2] = {0};

	// read temp data from BME280 via SPI
	HAL_GPIO_WritePin(bm.port, bm.pin, RESET); // chip select is active low
	HAL_SPI_Transmit (bm.spi, &address, 1, 1000);  // send address
	HAL_SPI_Receive (bm.spi, data_rec, 2, 1000);  // receive 6 bytes data
	HAL_GPIO_WritePin(bm.port, bm.pin, SET); // end transaction by pulling CSB high
	return data_rec[0];
}
