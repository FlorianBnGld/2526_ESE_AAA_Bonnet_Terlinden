/*
 * app.c
 *
 *  Created on: Nov 11, 2025
 *      Author: nicolas
 */

#include "app.h"
#include "user_interface/shell.h"
#include "tim.h"

#define ADC_MAX_VALUE 4095.0f
#define VREF_VOLTS 3.3f
#define OFFSET_VOLTS 1.65f
#define SENSITIVITY_V_PER_A 0.05f
#define CALIBRATION_SAMPLES 100
#define ADC_BUFFER_SIZE 16
static float g_calibrated_offset_volts = 1.65f;
extern ADC_HandleTypeDef hadc1;
static char shell_uart2_received_char;
volatile uint16_t adc_dma_buffer[ADC_BUFFER_SIZE];
volatile float g_current_amperes = 0.0f;



void init_device(void){
// Initialisation user interface
	// SHELL
	hshell1.drv.transmit = shell_uart2_transmit;
	hshell1.drv.receive = shell_uart2_receive;
	shell_init(&hshell1);
	HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1);

	// LED
	led_init();

	// BUTTON
//	button_init();
//
// Initialisation motor control
	// MOTOR
	motor_init();
	// ASSERV (PID)
//	asserv_init();
//
// Initialisation data acquistion
	// ANALOG INPUT
	analog_init();
	// ENCODER INPUT
//	input_encoder_init();
}


float calibrate_current_zero(void)
{
    // On s'assure que tout est arrêté avant de calibrer
    HAL_ADC_Stop_DMA(&hadc1);
    HAL_ADC_Stop(&hadc1);

    uint32_t total_raw = 0;

    // On lance l'ADC une seule fois
    HAL_ADC_Start(&hadc1);

    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
        {
            total_raw += HAL_ADC_GetValue(&hadc1);
        }
        HAL_Delay(1); // Petit délai pour laisser le signal se stabiliser
    }

    HAL_ADC_Stop(&hadc1);

    // Calcul de la moyenne
    float raw_mean = (float)total_raw / CALIBRATION_SAMPLES;

    // Mise à jour de l'offset global
    g_calibrated_offset_volts = (raw_mean / ADC_MAX_VALUE) * VREF_VOLTS;

    return g_calibrated_offset_volts;
}


void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (hadc->Instance == ADC1)
    {
        uint32_t sum = 0;
        for (uint8_t i = 0; i < ADC_BUFFER_SIZE; i++)
        {
            sum += adc_dma_buffer[i];
        }

        float adc_mean = (float)sum / ADC_BUFFER_SIZE;
        float u_out_volts = (adc_mean / ADC_MAX_VALUE) * VREF_VOLTS;

        g_current_amperes = (u_out_volts - g_calibrated_offset_volts) / SENSITIVITY_V_PER_A;
    }
}


float read_current_dma(void)
{
    // Il suffit de retourner la variable globale mise a jour par le DMA Callback
    return g_current_amperes;
}


void analog_init(void)
{
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
	calibrate_current_zero();
	start_adc_dma_acquisition();
	read_current_dma();
}





float read_current_polling()
{
    uint32_t adc_raw_value = 0;
    float u_out_volts = 0.0f;
    float imes_amperes = 0.0f;


    if (HAL_ADC_Start(&hadc1) != HAL_OK)
	{
		// Gerer l'erreur de demarrage
		return -999.0f;
	}

	if (HAL_ADC_PollForConversion(&hadc1, 100) != HAL_OK)
	{
		HAL_ADC_Stop(&hadc1);
		return -999.0f;
	}

    if (HAL_ADC_GetState(&hadc1) & HAL_ADC_STATE_REG_EOC)
    {
        adc_raw_value = HAL_ADC_GetValue(&hadc1);
    }

    HAL_ADC_Stop(&hadc1);

    // Uout = V_ADC * (Vref / (2^N - 1))
    u_out_volts = ((float)adc_raw_value / ADC_MAX_VALUE) * VREF_VOLTS;

    // Imes = (Uout - 1.65V) / (0.05V/A)
    imes_amperes = (u_out_volts - g_calibrated_offset_volts) / SENSITIVITY_V_PER_A;
    printf("Courant : %f A\r\n", imes_amperes);
    printf("Raw : %d \r\n", adc_raw_value);
    return imes_amperes;
}





int start_adc_dma_acquisition(void)
{
    if (HAL_ADC_Start_DMA(&hadc1,
                          (uint32_t*)adc_dma_buffer,
                          ADC_BUFFER_SIZE) != HAL_OK)
    {
        return -1;
    }

    HAL_TIM_Base_Start(&htim1);
    return 0;
}







uint8_t shell_uart2_transmit(const char *pData, uint16_t size)
{
	HAL_UART_Transmit(&huart2, (uint8_t *)pData, size, HAL_MAX_DELAY);
	return size;
}

uint8_t shell_uart2_receive(char *pData, uint16_t size)
{
	*pData = shell_uart2_received_char;
	return 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART2) {
		//		HAL_UART_Transmit(&huart2, (uint8_t *)&shell_uart2_received_char, 1, HAL_MAX_DELAY);
		HAL_UART_Receive_IT(&huart2, (uint8_t *)&shell_uart2_received_char, 1);
		shell_run(&hshell1);
	}
}





void loop(void)
{
    float current = read_current_dma();
    printf("Courant moteur : %.2f A\r\n", current);
    uint16_t raw = adc_dma_buffer[0];
    printf("RAW ADC = %d\r\n", raw);
    HAL_Delay(500);
}
