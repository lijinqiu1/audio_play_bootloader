/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2018 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"
#include "dma.h"
#include "fatfs.h"
#include "sdio.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include "oled.h"
#include "oledfont.h"
#include "bootloader.h"
#include "iap.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
uint8_t menu_index = 0;//记录当前选项 1：是 0：否
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */

/* USER CODE BEGIN 0 */
void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // This call can be used for debug purposes during application development.
    // @note CAUTION: Activating this code will write the stack to flash on an error.
    //                This function should NOT be used in a final product.
    //                It is intended STRICTLY for development/debugging purposes.
    //                The flash write will happen EVEN if the radio is active, thus interrupting
    //                any communication.
    //                Use with care. Un-comment the line below to use.
    // ble_debug_assert_handler(error_code, line_num, p_file_name);
	app_trace_log("0x%x %d %s\r\n", error_code,line_num,p_file_name);
    // On assert, the system can only recover with a reset.
    NVIC_SystemReset();
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */
  DIR update_dir;
  FIL update_file;
  FRESULT res;
  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */
   uint32_t last_tick = 0;
	uint8_t oled_flash_flag = 0;
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SDIO_SD_Init();
  MX_FATFS_Init();
  MX_UART7_Init();
  MX_SPI4_Init();

  /* USER CODE BEGIN 2 */
  OLED_Init();
  OLED_Clear();
  
  app_trace_log("begin!\n");
  HAL_Delay(1000);
  res = f_opendir(&update_dir,"0:/UPDATE");
  if (res != FR_OK)
  {
		app_trace_log("open dir faild!\n");
		res = f_mkdir("0:/UPDATE");
		if (res != FR_OK)
		{
		  app_trace_log("%s,%d,error %d\n",__FUNCTION__,__LINE__,res);
		  APP_ERROR_CHECK(res);
		}
		if(((*(volatile uint32_t*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
		{	 
			iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
		}
		else 
		{//无升级固件，请升级。
			while(1);
		}
  }
  else
  {
	  f_closedir(&update_dir);
	  res = f_open(&update_file,"0:/UPDATE/update.bin",FA_READ);
	  if (res != FR_OK)
	  {
		     if(((*(volatile uint32_t*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
			{	 
				app_trace_log("跳转到app\r\n");
				iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
			}
	  }
	  f_close(&update_file);
	  app_trace_log("open dir success!\n");
	  app_trace_log("open file success!\n");
	  OLED_ShowCHinese(8, 0, 0, (uint8_t (*)[32])HzUpdate);
	  OLED_ShowCHinese(24, 0, 1, (uint8_t (*)[32])HzUpdate);
	  OLED_ShowCHinese(40, 0, 2, (uint8_t (*)[32])HzUpdate);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */
	  if(HAL_GPIO_ReadPin(KEY_RIGHT_GPIO_Port,KEY_RIGHT_Pin) == GPIO_PIN_RESET)
	  {
		  if(menu_index == 1)
		  {
			  menu_index = 0;
		  }
	  }
	  if(HAL_GPIO_ReadPin(KEY_LEFT_GPIO_Port,KEY_LEFT_Pin) == GPIO_PIN_SET)
	  {
		  if(menu_index == 0)
		  {
			  menu_index = 1;
		  }
	  }
	  if ((HAL_GetTick() - last_tick) > 1000)
	  {
		  last_tick = HAL_GetTick();
		  if (menu_index == 1)
		  {
			  /*是*/
			  if(oled_flash_flag)
			  {
				  oled_flash_flag = 0;
				  OLED_ShowCHinese(8, 4, 0, (uint8_t (*)[32])HzBool);
			  }
			  else
			  {
				  oled_flash_flag = 1;
				  OLED_ShowCHinese(8, 4, 0, (uint8_t (*)[32])HzBool1);
			  }
			  /*否*/
			  OLED_ShowCHinese(40, 4, 1, (uint8_t (*)[32])HzBool);
		  }
	      else
		  {
			  /*否*/
			  if(oled_flash_flag)
			  {
				  oled_flash_flag = 0;
				  OLED_ShowCHinese(40, 4, 1, (uint8_t (*)[32])HzBool);
			  }
			  else
			  {
				  oled_flash_flag = 1;
				  OLED_ShowCHinese(40, 4, 1, (uint8_t (*)[32])HzBool1);
			  }
			  /*是*/
			  OLED_ShowCHinese(8, 4, 0, (uint8_t (*)[32])HzBool);
		  }
			
	  }
	  if (HAL_GPIO_ReadPin(KEY_REPORT_GPIO_Port,KEY_REPORT_Pin)==GPIO_PIN_SET)
	  {
			if (menu_index == 1)
			{
				FirmwareUpdate();
				if(((*(volatile uint32_t*)(FLASH_APP1_ADDR+4))&0xFF000000)==0x08000000)//判断是否为0X08XXXXXX.
				{	 
					app_trace_log("固件升级成功，准备跳转到App程序处开始执行!!\r\n");
					OLED_Clear();
					OLED_ShowCHinese(8, 0, 0, (uint8_t (*)[32])HzComplete);
					OLED_ShowCHinese(40, 0, 1, (uint8_t (*)[32])HzComplete);
					res = f_unlink("0:/UPDATE/update.bin");
					if (res != FR_OK)
					{
						app_trace_log("rm file faild %d",res);
					}
					HAL_Delay(1000);
					iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
				}
				else 
				{
					app_trace_log("固件升级失败!\r\n");	   
				}
			}
			else
			{
				iap_load_app(FLASH_APP1_ADDR);//执行FLASH APP代码
			}
	  }
  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: app_trace_log("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
