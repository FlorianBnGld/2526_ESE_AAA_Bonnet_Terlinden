/*
 * motor.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/motor.h"



int motor_init(){
	return shell_add(&hshell1, "motor", motor_control, "Control MOTOR");
}

int motor_control(h_shell_t* h_shell, int argc, char** argv)
{
	int size;
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);


	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, ((0.6)*4250));

	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, ((1-0.6)*4250));

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "PWM ON\r\n");
	h_shell->drv.transmit(h_shell->print_buffer, size);
}

