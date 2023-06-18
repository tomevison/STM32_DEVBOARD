/*
 * piezo.c
 *
 *  Created on: Jun 12, 2023
 *      Author: me
 */
#include "piezo.h"

HAL_StatusTypeDef PIEZO_setHz(double g, TIM_HandleTypeDef *timer){

	//      Hz
	// C3	130.81
	// C4	261.63
	// C5	523.25

	// input should range from 0-1000g
	// we wanto scale this from 131Hz to 523Hz

	// preload from 61068.70229 >> 15037.59398
	//double low = 61068.702; // low frequency C3
	double low = 30651.34; // low frequency C4
	double hig = 15037.594; // high frequency C5

	// limit the input acceleration
	if(g>1000){
		g = 1000;
	}else if(g<=0){
		g = 0;
	}

	// scale to frequency
	double prelod = map(g, 0,1000,hig,low);
	double dutyCycle = prelod/2;
	//__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, dutyCycle);
	//TIM1->ARR = (uint32_t)prelod;
	//TIM1->CCR3 = (uint32_t)dutyCycle;
	__HAL_TIM_SET_AUTORELOAD(timer,(uint32_t)prelod); // 31khz
	__HAL_TIM_SET_COMPARE(timer,TIM_CHANNEL_3,(uint32_t)dutyCycle);

	return HAL_OK;
}

double map(double x, double in_min, double in_max, double out_min, double out_max){
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

