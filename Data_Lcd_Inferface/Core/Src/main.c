/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define LCD_DATA_PORT GPIOD
#define LCD_CTRL_PORT GPIOE
#define LCD_RS_PIN    GPIO_PIN_0
#define LCD_E_PIN     GPIO_PIN_1
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void LCD_Delay_us(uint32_t us);
void LCD_EnablePulse(void);
void LCD_SendCommand(uint8_t cmd);
void LCD_SendData(uint8_t data);
void LCD_Init(void);
void LCD_SendString(const char *str);
void LCD_SetCursor(uint8_t row, uint8_t col);

#define ADXL345_I2C_ADDR      (0x53 << 1) // 0xA6
#define ADXL345_REG_POWER_CTL  0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0     0x32

void ADXL345_Init(void);
void ADXL345_Read_Accel(int16_t *x, int16_t *y, int16_t *z);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void LCD_Delay_us(uint32_t us) {
    uint32_t count = us * (SystemCoreClock / 1000000U) / 5U;
    while(count--) { __NOP(); }
}

void LCD_EnablePulse(void) {
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_SET);
    LCD_Delay_us(10);
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_E_PIN, GPIO_PIN_RESET);
    LCD_Delay_us(50);
}

void LCD_SendCommand(uint8_t cmd) {
    // RS = 0 for Command
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_RESET);

    // Write lower byte to PD0-PD7
    LCD_DATA_PORT->ODR = (LCD_DATA_PORT->ODR & 0xFF00) | cmd;

    LCD_EnablePulse();

    // Commands like Clear Display or Return Home need ~2ms to execute
    if (cmd == 0x01 || cmd == 0x02) {
        HAL_Delay(2);
    }
}

void LCD_SendData(uint8_t data) {
    // RS = 1 for Data
    HAL_GPIO_WritePin(LCD_CTRL_PORT, LCD_RS_PIN, GPIO_PIN_SET);

    // Write lower byte to PD0-PD7
    LCD_DATA_PORT->ODR = (LCD_DATA_PORT->ODR & 0xFF00) | data;

    LCD_EnablePulse();
}

void LCD_Init(void) {
    // Wait >15ms after power-on
    HAL_Delay(50);

    // Function set: 8-bit mode, 2 lines, 5x8 font
    LCD_SendCommand(0x38);
    HAL_Delay(5);

    LCD_SendCommand(0x38);
    LCD_Delay_us(150);

    LCD_SendCommand(0x38);

    // Display ON, Cursor OFF, Blink OFF
    LCD_SendCommand(0x0C);

    // Clear Display
    LCD_SendCommand(0x01);

    // Entry Mode Set: Increment cursor, no shift
    LCD_SendCommand(0x06);
}

void LCD_SendString(const char *str) {
    while (*str) {
        LCD_SendData((uint8_t)(*str++));
    }
}

void LCD_SetCursor(uint8_t row, uint8_t col) {
    uint8_t address = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_SendCommand(address);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  /* USER CODE BEGIN 2 */
  // Initialize LCD controller in 8-bit mode
  LCD_Init();

  // Write text on Line 1
  LCD_SetCursor(0, 0);
  LCD_SendString(" INTEGRATION OF ");

  // Write text on Line 2
  LCD_SetCursor(1, 0);
  LCD_SendString("16x2 WITH STM32 ");


  LCD_Init();
  ADXL345_Init();


  uint8_t dev_id = 0;
  HAL_I2C_Mem_Read(&hi2c1, ADXL345_I2C_ADDR, 0x00, 1, &dev_id, 1, 100);

  if (dev_id == 0xE5) {
      LCD_SetCursor(0, 0);
      LCD_SendString("Sensor Connected");
      HAL_Delay(1000);
  } else {
      char err_buf[17];
      snprintf(err_buf, sizeof(err_buf), "Err ID: 0x%02X", dev_id);
      LCD_SetCursor(0, 0);
      LCD_SendString(err_buf);
      while(1); // Stop execution to signal setup fault
  }

  LCD_SetCursor(0, 0);
  LCD_SendString(" ADXL345 Ready ");
  HAL_Delay(1000);
  LCD_SendCommand(0x01); // Clear screen


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

	  int16_t x_raw = 0, y_raw = 0, z_raw = 0;
	      float x_g = 0.0, y_g = 0.0, z_g = 0.0;
	      char buf[17];

	      ADXL345_Read_Accel(&x_raw, &y_raw, &z_raw);

	      // Convert raw counts to g-force
	      x_g = x_raw * 0.0039f;
	      y_g = y_raw * 0.0039f;
	      z_g = z_raw * 0.0039f;

	      // Line 1: X and Y readings
	      snprintf(buf, sizeof(buf), "X:%.2f  Y:%.2f", x_g, y_g);
	      LCD_SetCursor(0, 0);
	      LCD_SendString(buf);

	      // Line 2: Z reading
	      snprintf(buf, sizeof(buf), "Z:%.2f g        ", z_g);
	      LCD_SetCursor(1, 0);
	      LCD_SendString(buf);

	      HAL_Delay(200);
	      /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 50;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_0|GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pins : PD0 PD1 PD2 PD3
                           PD4 PD5 PD6 PD7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PE0 PE1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE BEGIN 4 */
void ADXL345_Init(void) {
    uint8_t data = 0x01; // +-4g range, full resolution
    HAL_I2C_Mem_Write(&hi2c1, ADXL345_I2C_ADDR, ADXL345_REG_DATA_FORMAT, 1, &data, 1, HAL_MAX_DELAY);

    data = 0x08; // Enable measurement mode
    HAL_I2C_Mem_Write(&hi2c1, ADXL345_I2C_ADDR, ADXL345_REG_POWER_CTL, 1, &data, 1, HAL_MAX_DELAY);
}

void ADXL345_Read_Accel(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t buffer[6];
    if (HAL_I2C_Mem_Read(&hi2c1, ADXL345_I2C_ADDR, ADXL345_REG_DATAX0,
                             I2C_MEMADD_SIZE_8BIT, buffer, 6, 100) == HAL_OK)
        {
            *x = (int16_t)((buffer[1] << 8) | buffer[0]);
            *y = (int16_t)((buffer[3] << 8) | buffer[2]);
            *z = (int16_t)((buffer[5] << 8) | buffer[4]);
        }
    }
/* USER CODE END 4 */

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
