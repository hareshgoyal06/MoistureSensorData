#include "main.h"
#include <stdio.h>
#include <math.h> // For log and pow functions

/* Global Variables */
ADC_HandleTypeDef hadc1;
UART_HandleTypeDef huart1;

/* Function Prototypes */
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_USART1_UART_Init(void);
float calculateTemperature(uint32_t adcValue);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_USART1_UART_Init();

    uint32_t adcValueMoisture = 0;
    uint32_t adcValueThermistor = 0;
    uint16_t moisturePercentage = 0;
    float temperature = 0.0;
    char uartBuffer[50];

    while (1)
    {
    	 // Read Moisture Sensor (ADC Channel 0)
    	        ADC_ChannelConfTypeDef sConfig = {0};
    	        sConfig.Channel = ADC_CHANNEL_0;
    	        sConfig.Rank = 1;
    	        sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    	        HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    	        HAL_ADC_Start(&hadc1);
    	        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    	        adcValueMoisture = HAL_ADC_GetValue(&hadc1);
    	        HAL_ADC_Stop(&hadc1);

    	        // Convert ADC value to percentage
    	        moisturePercentage = (adcValueMoisture * 100) / 4095;

    	        // Read Thermistor (ADC Channel 1)
    	        sConfig.Channel = ADC_CHANNEL_1;
    	        HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    	        HAL_ADC_Start(&hadc1);
    	        HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    	        adcValueThermistor = HAL_ADC_GetValue(&hadc1);
    	        HAL_ADC_Stop(&hadc1);

    	        // Convert ADC value to temperature
    	        temperature = calculateTemperature(adcValueThermistor);

    	        // Format and transmit data via UART
    	        // Only send numerical values separated by a comma
    	        snprintf(uartBuffer, sizeof(uartBuffer), "%.2f,%u\n", temperature, moisturePercentage);
    	        HAL_UART_Transmit(&huart1, (uint8_t *)uartBuffer, strlen(uartBuffer), HAL_MAX_DELAY);

		HAL_Delay(100); // Transmit data every second
    }
}

/* Function to Calculate Temperature */
float calculateTemperature(uint32_t adcValue)
{
    float voltage = (adcValue * 3.3f) / 4095;
    float resistance = (10000.0f * (3.3f - voltage)) / voltage;
    float temperature = 1.0 / (0.001129148 + (0.000234125 * log(resistance)) + (0.0000000876741 * pow(log(resistance), 3))) - 273.15;
    return temperature;
}
/* ADC1 Initialization Function */
static void MX_ADC1_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* Configure the global features of the ADC (Clock, Resolution, Data Alignment, and number of conversion) */
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = DISABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion = 1;
    hadc1.Init.DMAContinuousRequests = DISABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* Configure ADC regular channel for Moisture Sensor (default) */
    sConfig.Channel = ADC_CHANNEL_0;  // PA0 (ADC1_IN0)
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/* USART1 Initialization Function */
static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
}

/* GPIO Initialization Function */
static void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOA_CLK_ENABLE();

    /* Configure GPIO pin for onboard LED (PA5) */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    /* Configure GPIO pins: PA6 and PA7 */
    GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7; // PA6 for LED1, PA7 for LED2
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;   // Push-pull output
    GPIO_InitStruct.Pull = GPIO_NOPULL;           // No pull-up or pull-down
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;  // Low frequency
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

/* Error Handler */
void Error_Handler(void)
{
    while (1)
    {
    }
}
