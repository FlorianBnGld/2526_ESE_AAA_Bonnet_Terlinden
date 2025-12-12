/*
 * motor.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "motor_control/motor.h"
#define alpha 0.6
#define alpha_50 50
#define ARR 4250

int motor_init(){
    int ret = 0;
	ret |= shell_add(&hshell1, "motor", motor_control, "Regler le rapport cyclique du moteur (ex: motor 50)");
	ret |= shell_add(&hshell1, "start", motor_start, "Demarrer le moteur a 50%% de rapport cyclique");
	ret |= shell_add(&hshell1, "stop", motor_stop, "Arreter la generation PWM du moteur");
	return ret;
}

int motor_control(h_shell_t* h_shell, int argc, char** argv)
{
	int size;

	//Sécurité
	if (argc < 2) {
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Erreur: Entrez un rapport (0-100)\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}
	int alpha_input = atoi(argv[1]);

	//Clamping

	if (alpha_input > 100) {
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Erreur: Entrez un rapport (0-100)\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}
	if (alpha_input < 0) {
		size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Erreur: Entrez un rapport (0-100)\r\n");
		h_shell->drv.transmit(h_shell->print_buffer, size);
		return -1;
	}


	uint32_t pulse_ch1 = (alpha_input * ARR) / 100;
	uint32_t pulse_ch2 = ((100 - alpha_input) * ARR) / 100;

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
	HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);


	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_ch1);
	__HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_ch2);

	size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE,
			"PWM Set: CH1=%d%% (%lu), CH2=%d%% (%lu)\r\n",
			alpha_input, pulse_ch1, (100-alpha_input), pulse_ch2);
	h_shell->drv.transmit(h_shell->print_buffer, size);

	return 0;
}

int motor_start(h_shell_t* h_shell, int argc, char** argv)
{
	int size;
    int pulse_50_percent = (alpha_50 * ARR) / 100;

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);

    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_2);

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, pulse_50_percent);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, pulse_50_percent);

    size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Moteur démarré : PWM CH1/CH2 initialisé à 50%%.\r\n");
    h_shell->drv.transmit(h_shell->print_buffer, size);
    return 0;
}


int motor_stop(h_shell_t* h_shell, int argc, char** argv)
{
	int size;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);

    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim1, TIM_CHANNEL_2);
    HAL_TIMEx_PWMN_Stop(&htim1, TIM_CHANNEL_2);

    size = snprintf(h_shell->print_buffer, SHELL_PRINT_BUFFER_SIZE, "Moteur arrêté : PWM CH1/CH2 stoppé.\r\n");
    h_shell->drv.transmit(h_shell->print_buffer, size);
    return 0;
}
