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
#define ADC_BUFFER_SIZE 1
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
    float total_u_out = 0.0f;
    uint32_t adc_raw_value;
    float u_out_volts;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++)
    {
        if (HAL_ADC_Start(&hadc1) == HAL_OK)
        {
            if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
            {
                if (HAL_ADC_GetState(&hadc1) & HAL_ADC_STATE_REG_EOC)
                {
                    adc_raw_value = HAL_ADC_GetValue(&hadc1);
                    u_out_volts = ((float)adc_raw_value / ADC_MAX_VALUE) * VREF_VOLTS;
                    total_u_out += u_out_volts;
                }
            }
            HAL_ADC_Stop(&hadc1);
        }
    }
    g_calibrated_offset_volts = total_u_out / CALIBRATION_SAMPLES;
    printf("Courant de calibration : %f A\r\n", g_calibrated_offset_volts);
    return g_calibrated_offset_volts;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if(hadc->Instance == ADC1)
    {
        uint32_t adc_raw_value;
        float u_out_volts;
        adc_raw_value = adc_dma_buffer[0];
        u_out_volts = ((float)adc_raw_value / ADC_MAX_VALUE) * VREF_VOLTS;
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
    if (HAL_TIM_Base_Start(&htim1) != HAL_OK)
    {
        return -1;
    }

    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, ADC_BUFFER_SIZE) != HAL_OK)
    {
        HAL_TIM_Base_Stop(&htim1);
        return -1;
    }

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





void loop(){
	read_current_polling();
	HAL_Delay(500); // Lire toutes les 500 ms

}
