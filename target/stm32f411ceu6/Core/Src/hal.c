#include "main.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "hal.h"

GPIO_TypeDef *STM32F4_Registers[] = {
    GPIOA,
    GPIOB,
    GPIOC,
    GPIOD,
    GPIOE,
    GPIOH,
};

void SystemClock_Config(void);

int MCU_Init()
{
    HAL_StatusTypeDef hal_init_status = HAL_Init();
    SystemClock_Config();
    if (hal_init_status == HAL_OK)
        return MCU_INIT_OK;
    return MCU_INI_ERR;
}

int GPIO_Init(int gpioPort, int gpioPin)
{
    if (gpioPort < PORTS_NUM)
    {
        GPIO_InitTypeDef GPIO_InitStruct = {0};

        /* Enable clock for the given pin*/
        RCC->AHB1ENR |= (1 << gpioPort);

        /*Configure GPIO pin */
        GPIO_InitStruct.Pin = (1 << gpioPin);
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        HAL_GPIO_Init(STM32F4_Registers[gpioPort], &GPIO_InitStruct);

        return 0;
    }
    return 1;
}

int GPIO_Set(int gpioPort, int gpioPin, int value)
{
    if (gpioPort < PORTS_NUM)
    {
        HAL_GPIO_WritePin(STM32F4_Registers[gpioPort], (1 << gpioPin), value);
        return 0;
    }

    return 1;
}

void MCU_Delay(int miliseconds)
{
    HAL_Delay(miliseconds);
}
int USB_Init()
{
    MX_USB_DEVICE_Init();
    return 0;
}

int USB_TX(char *buffer, int bufferSize)
{
    CDC_Transmit_FS(buffer, bufferSize);

    return 0;
}
/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
    /* USER CODE END Error_Handler_Debug */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /** Configure the main internal regulator output voltage
     */
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /** Initializes the RCC Oscillators according to the specified parameters
     * in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 192;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 4;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
    {
        Error_Handler();
    }
}