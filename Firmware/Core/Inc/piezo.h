/*
 * piezo.h
 *
 *  Created on: Jun 12, 2023
 *      Author: me
 */

#ifndef INC_PIEZO_H_
#define INC_PIEZO_H_

#include "stm32f4xx_hal.h" // cubeIDE HAL is used

HAL_StatusTypeDef PIEZO_setHz(double g, TIM_HandleTypeDef *timer);
double map(double x, double in_min, double in_max, double out_min, double out_max);
#endif /* INC_PIEZO_H_ */
