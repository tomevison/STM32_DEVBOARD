

uint32_t BME280_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, SPI_HandleTypeDef* hspi1);
BME280_setMode();
BME280_setTempOS();
BME280_setPresOS();
BME280_setHumdOS();
uint32_t BME280_getHumd();
